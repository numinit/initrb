#include <initrb.h>
#include <initrb/platform.h>

#include <sys/types.h>
#include <sys/wait.h>

static void initrb_reap(int signal) {
    assert(signal == SIGCHLD);
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

static VALUE initrb_interpreter_boot(VALUE file) {
    int exc = 0;
    rb_load_protect(file, Qfalse, &exc);
    if (exc) {
        // Caught by the rescue clause
        rb_jump_tag(exc);
    } else {
        // Script exited
        return INT2FIX(EXIT_SUCCESS);
    }
}

static VALUE initrb_interpreter_eval_impl(VALUE args) {
    Check_Type(args, T_ARRAY);
    VALUE str = rb_ary_entry(args, 0);
    VALUE binding = rb_ary_entry(args, 1);
    VALUE filename = rb_ary_entry(args, 2);
    VALUE line = rb_ary_entry(args, 3);
    return rb_funcall(rb_mKernel, rb_intern("eval"), 4, str, binding, filename, line);
}

static VALUE initrb_interpreter_eval(VALUE args) {
    int exc = 0;
    VALUE ret = rb_protect(initrb_interpreter_eval_impl, args, &exc);
    if (exc) {
        rb_jump_tag(exc);
    } else {
        return ret;
    }
}

static VALUE initrb_interpreter_inspect_impl(VALUE obj) {
    return rb_funcallv_public(obj, rb_intern("inspect"), 0, NULL);
}

static VALUE initrb_interpreter_inspect(VALUE obj) {
    int exc = 0;
    VALUE ret = rb_protect(initrb_interpreter_inspect_impl, obj, &exc);
    if (exc) {
        rb_jump_tag(exc);
    } else {
        return ret;
    }
}

static VALUE initrb_interpreter_begin(VALUE file) {
    /** `file' may be a symlink or a relative path, expand it and dereference all symlinks **/
    VALUE real_name = rb_funcall(rb_cFile, rb_intern("realpath"), 1, file);
    VALUE dir = rb_file_dirname(real_name);
    VALUE ret;
    int exc = 0;

    /** Add the file's directory to the library search path **/
    rb_ary_unshift(rb_gv_get(":"), dir);

    /** Run the interpreter */
    ret = rb_protect(initrb_interpreter_boot, real_name, &exc);
    if (exc) {
        rb_jump_tag(exc);
    }

    return ret;
}

static VALUE initrb_interpreter_print_exception(VALUE exception) {
    VALUE arr = rb_ary_new3(3, rb_obj_as_string(rb_obj_class(exception)), rb_obj_as_string(exception), rb_ary_join(rb_funcall(exception, rb_intern("backtrace"), 0), rb_str_new_cstr("\n\tfrom ")));
    return rb_funcall(rb_stderr, rb_intern("puts"), 1, rb_str_format(RARRAY_LENINT(arr), RARRAY_PTR(arr), rb_str_new_cstr("%s: %s\nfrom %s")));
}

static VALUE initrb_interpreter_print_result(VALUE result) {
    VALUE arr = rb_ary_new3(1, rb_obj_as_string(result));
    return rb_funcall(rb_stdout, rb_intern("puts"), 1, rb_str_format(RARRAY_LENINT(arr), RARRAY_PTR(arr), rb_str_new_cstr("=> %s")));
}

static VALUE initrb_interpreter_rescue(VALUE args, VALUE exception) {
    initrb_interpreter_print_exception(exception);
    return Qundef;
}

static VALUE initrb_interpreter_rescue_wrapper(initrb_ruby_fn_t begin, VALUE args) {
    return rb_rescue(begin, args, initrb_interpreter_rescue, args);
}

static void initrb_repl() {
    char prompt[128];
    int idx = 0;

    printf("*** DROPPING YOU INTO A REPL ***\n\n");
    while (true) {
        VALUE arr, result, inspected;
        char *line;

        snprintf(prompt, sizeof(prompt), "initrb:%03d> ", ++idx);
        line = readline(prompt);

        if (!line) {
            break;    
        } else {
            // Evaluate the code
            arr = rb_ary_new3(4, rb_str_buf_new_cstr(line), rb_const_get(rb_cObject, rb_intern("TOPLEVEL_BINDING")), rb_str_buf_new_cstr("(initrb)"), INT2FIX(idx));
            result = initrb_interpreter_rescue_wrapper(initrb_interpreter_eval, arr);
            if (result == Qundef) {
                continue;
            }
            
            // Inspect the result
            inspected = initrb_interpreter_rescue_wrapper(initrb_interpreter_inspect, result);
            if (inspected == Qundef) {
                continue;
            }

            // Print the result
            initrb_interpreter_print_result(inspected);

            // Free the line
            free(line);
        }
    }
}

static int initrb_boot(int argc, char **argv) {
    int ret = 0;
    while (true) {
        ret = initrb_interpreter_rescue_wrapper(initrb_interpreter_begin, rb_str_buf_new_cstr(argv[argc - 1]));
        if (ret != Qundef) {
            // Exit gracefully
            break;
        } else {
            // Internal error, drop into the REPL
            initrb_repl();
            break;
        }
    }
    return ret;
}

int main(int argc, char **argv) {
    sigset_t set;
    initrb_signal_handler_t handlers[] = {
        {.signal = SIGCHLD, .handler = initrb_reap},
        {.signal = SIGINT,  .handler = SIG_IGN},
        {.signal = SIGUSR1, .handler = SIG_IGN}
    };

    // Initial setup
    if (argc != 2) {
        fprintf(stderr, "usage %s <script.rb>\n", argv[0]);
        return EXIT_FAILURE;
    } 
    /*if (getpid() != 1 || geteuid() != 0) {
        fprintf(stderr, "You must run this as root using PID 1.\n");
        return EXIT_FAILURE;
    } else */else if (chdir("/") != 0) {
        fprintf(stderr, "chdir failure\n");
        return EXIT_FAILURE;
    } else if (sigfillset(&set) != 0) {
        fprintf(stderr, "sigfillset failure\n");
        return EXIT_FAILURE;
    }

    /* Install signal handlers */
    for (size_t i = 0; i < INITRB_COUNT(handlers); i++) {
        struct sigaction action;

        // Restart syscalls if we get a signal
        action.sa_flags = SA_RESTART;

        // Set the handler to the handler, or SIG_IGN as a default
        action.sa_handler = handlers[i].handler != NULL ? handlers[i].handler : SIG_IGN;

        // Register the action
        if (sigaction(handlers[i].signal, &action, NULL) != 0) {
            fprintf(stderr, "sigaction failure\n");
            return EXIT_FAILURE;
        }

        // Remove it from the set of signals to block
        if (sigdelset(&set, handlers[i].signal) != 0) {
            fprintf(stderr, "sigdelset failure\n");
            return EXIT_FAILURE;
        }
    }

    // Set the process signal mask
    if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
        fprintf(stderr, "sigprocmask failure\n");
    }

    return initrb_start(argc, argv, initrb_boot);
}
