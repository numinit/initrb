#include <initrb/platform.h>

#include <ruby.h>

static void initrb_boot_statics(void) {
    VALUE rb_mInit = rb_define_module("RBInit");
    rb_define_global_const("Init", rb_mInit);
}

int initrb_start(int argc, char **argv, initrb_boot_fn_t boot) {
    int ret = 0;

    {
        /* Initialize the stack */
        RUBY_INIT_STACK;

        /* Initialize the interpreter */
        ruby_init();

        /* Initialize the loadpath */
        ruby_init_loadpath();

        /* We're initrb */
        ruby_script("initrb");

        /* Set the argv */
        ruby_set_argv(argc, argv);

        /* Boot statics */
        initrb_boot_statics();

        /* Run */
        ret = boot(argc, argv);

        /* Finalize the interpreter */
        ruby_finalize();
    }

    return ret;
}
