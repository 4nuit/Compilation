#include <gcc-plugin.h>
#include <tree.h>
#include <tree-pass.h>
#include <context.h>
#include <function.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <basic-block.h>

void print_blocs_index(function *fun){
    basic_block bb;
    gimple_stmt_iterator gsi;
    gimple *stmt;

    FOR_EACH_BB_FN(bb,fun){
        gsi = gsi_start_bb(bb);
        stmt = gsi_stmt(gsi);
        printf("          |||++||| BLOCK INDEX %d\n", bb->index);
	}
}

const pass_data my_pass_data = {
    GIMPLE_PASS, /* type */
    "NEW_PASS", /* name */
    OPTGROUP_NONE, /* optinfo_flags */
    TV_OPTIMIZE, /* tv_id */
    0, /* properties_required */
    0, /* properties_provided */
    0, /* properties_destroyed */
    0, /* todo_flags_start */
    0, /* todo_flags_finish */
};

class my_pass : public gimple_opt_pass {
    public:
        my_pass (gcc::context *ctxt)
            :gimple_opt_pass (my_pass_data, ctxt)
        {}

        my_pass *clone () { return new my_pass(g); }

        bool gate (function *fun) { 
            return true; }

        unsigned int execute (function *fun) {
            printf("Executing my_pass with function %s\n", function_name(fun) );
            print_blocs_index(fun);
            return 0;
        }
};

int plugin_is_GPL_compatible;

int plugin_init(struct plugin_name_args * plugin_info,struct plugin_gcc_version * version){
    struct register_pass_info pass_info;
    my_pass p(g);
    pass_info.pass = &p;
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 0;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    register_callback(plugin_info->base_name,PLUGIN_PASS_MANAGER_SETUP,NULL,&pass_info);
    return 0;
}