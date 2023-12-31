#include "../headers/plugin.h"
#include "mpi_collectives.cpp"
#include "cfgviz.cpp"
#include "frontiers.cpp"
#include "pragma.cpp"

int plugin_is_GPL_compatible;

const pass_data my_pass_data =
{
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

/* My new pass inheriting from regular gimple pass */
class my_pass : public gimple_opt_pass
{
	public:
		my_pass (gcc::context *ctxt)
			: gimple_opt_pass (my_pass_data, ctxt)
		{}

		my_pass *clone ()
		{
			return new my_pass(g);
		}

		bool gate (function *fun)
		{
			//function_isol_print(fun);
			printf("plugin: gate... \n");
			return true;
		}

		unsigned int execute (function *fun)
		{
			printf("plugin: execute...\n");
			printf("Function: '%s'\n", function_name(fun));

			/*****PDF+ Warnings ******/
			basic_block bb = NULL;
			split_on_mpi_collectives(bb,fun);
			calculate_dominance_info(CDI_POST_DOMINATORS);

			printf("PDF Basic Blocks\n");			
			bitmap_head *frontiers = bitmap_init();
			post_dom_frontiers(fun,frontiers);
			cfgviz_dump(fun,"postdomfront");

			printf("CFG2 - Loop Removed\n");
			bitmap_head *all_preds = bitmap_init();
			removeloop_cfg2(fun,all_preds);
			cfgviz_dump(fun,"removeloop");

			//bitmap_head *all_preds = bitmap_init();
			//bb = ENTRY_BLOCK_PTR_FOR_FN(fun)->next_bb;
			//mark_edge(bb);

			free_dominance_info(CDI_POST_DOMINATORS);
			clear_all_bb_aux(fun);

		return 0;
	}
}; 

void gracefull_exit_handler(__attribute__((unused)) void *gcc_data, __attribute__((unused)) void *user_data) {
    // Warns if some function was in pragmas but not in actual code
    int index;
    if(!mpi_pragmas::seen_in_pragmas.is_empty()){
        pragma_el* el = &mpi_pragmas::seen_in_pragmas.pop();
		// rich_location allows multiple lines to appear in a single warning, error, debug, etc ...
        rich_location richloc(line_table, el->loc);
        richloc.add_fixit_remove(el->loc);
        FOR_EACH_VEC_ELT(mpi_pragmas::seen_in_pragmas, index, el) {
            richloc.add_range(el->loc, SHOW_RANGE_WITH_CARET);
            richloc.add_fixit_remove(el->loc);
        }
        warning_at(&richloc, 0, "Function names found in pragmas but not in actual code");
    }
};


/* Main entry point for plugin */
int plugin_init(struct plugin_name_args * plugin_info, struct plugin_gcc_version * version)
{
	struct register_pass_info my_pass_info;
    printf( "plugin_init: Entering...\n" ) ;

	if(!plugin_default_version_check(version, &gcc_version)) 
		return 1;

	printf( "plugin_init: Check ok...\n" ) ;
	my_pass p(g);

	/* Fill info on my pass 
	 (insertion after the pass building the CFG) */
	my_pass_info.pass = &p;
	my_pass_info.reference_pass_name = "cfg";
	my_pass_info.ref_pass_instance_number = 0;
	my_pass_info.pos_op = PASS_POS_INSERT_AFTER;

	/* Add my pass to the pass manager */
	register_callback(plugin_info->base_name, 
			PLUGIN_PASS_MANAGER_SETUP, 
			NULL, 
			&my_pass_info);

	printf( "plugin_init: Pass added...\n" ) ;

	/* Add my PRAGME pass to the pass manager */
	register_callback(plugin_info->base_name, 
			PLUGIN_PRAGMAS, 
			mpi_pragmas::my_callback_mpicoll_register, 
			NULL);

	printf( "plugin_pragma: Pass added...\n" );

    register_callback(
            plugin_info->base_name,
            PLUGIN_FINISH,
            &gracefull_exit_handler,
            nullptr);

	printf( "plugin_pragma: Pragmas registered !\n");
	return 0;
}


