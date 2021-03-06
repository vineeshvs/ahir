#include <AaProgram.h>
#include <Aa2VC.h>

// for the pipelined case.
void AaStatement::Write_VC_RAW_Release_Dependencies(AaExpression* expr, set<AaRoot*>& visited_elements)
{
  set<AaRoot*> non_triv_preds;
  expr->Identify_Non_Trivial_Predecessors(non_triv_preds, visited_elements);
  Write_VC_RAW_Release_Deps(((AaRoot*)this),non_triv_preds);
}

void AaStatement::Write_VC_Synch_Dependency(set<AaRoot*>& visited_elements, bool pipeline_flag, ostream& ofile)
{
	if(this->_synch_statements.size() > 0)
	{
		for(set<AaStatement*>::iterator siter = _synch_statements.begin(), fiter = _synch_statements.end();
				siter != fiter; siter++)
		{
			AaStatement* stmt = *siter;
			if(visited_elements.find(stmt) != visited_elements.end())
			{
				ofile << "// forced synch" << endl;
				__J(__SST(this), __UCT(stmt));
			}
		}
	}
}

// a lot of code repetition, but can it be avoided?

void AaStatementSequence::Write_VC_Control_Path_Optimized(ostream& ofile)
{
	for(int idx = 0, fidx = _statement_sequence.size(); idx < fidx; idx++)
	{
		_statement_sequence[idx]->Write_VC_Control_Path_Optimized(ofile);
	}
}


void AaStatementSequence::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
	for(int idx = 0, fidx = _statement_sequence.size(); idx < fidx; idx++)
	{
		_statement_sequence[idx]->Write_VC_Links_Optimized(hier_id,ofile);
	}
}

// AaNullStatement
void AaNullStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
	this->Write_VC_Control_Path(ofile);
}

// AaAssignmentStatement
void AaAssignmentStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
	// this should never be called.
	assert(0);
}

void AaAssignmentStatement::Write_VC_WAR_Dependencies(bool pipeline_flag,
		set<AaRoot*>& visited_elements,
		ostream& ofile)
{

	if(!this->Is_Constant())
	{
		AaExpression* tgt = this->_target;

		if(tgt->Is_Implicit_Variable_Reference())
			tgt->Write_VC_WAR_Dependencies(pipeline_flag,
					visited_elements,this->_source,ofile);
	}

}


// New addition: the pipeline-flag.
// In this case, in addition to the data-dependency, the following pipeline 
// release dependencies is added.
// 
//    the source expression can be updated as soon
//    as the target has used the previous result of the source.
//
//    the guard expression can be updated as soon
//    as the statement has completed.
//
//
//   forward dependencies
//            guard-complete ->  statement-start.
//            source-complete -> statement-active.
//            statement-active -> target-start.
//            target-complete  -> statement-complete.
//            statement-start  -> statement-active  (could be redundant)
//            statement-active -> statement-complete (could be redundant)
//
//   backward dependencies.
//            statement-active -> guard-reenable.
//            statement-complete -> statement-active (could be redundant).
void AaAssignmentStatement::Write_VC_Control_Path_Optimized(bool pipeline_flag, 
		set<AaRoot*>& visited_elements,
		map<string, vector<AaExpression*> >& ls_map,
		map<string, vector<AaExpression*> >& pipe_map,
		AaRoot* barrier,
		ostream& ofile)
{
	if(!this->Is_Constant())
	{

		ofile << "// " << this->To_String() << endl;
		ofile << "// " << this->Get_Source_Info() << endl;


      // take care of the guard
      if(this->_guard_expression)
	{
	  if(this->_guard_expression->Is_Constant())
	  {
       	  	ofile << "// Guard expression is a constant" << endl;
	  }
	  else
	  {
	  	// guard expression calculation
       	  	ofile << "// Guard expression " << endl;
	  	this->_guard_expression->Write_VC_Control_Path_Optimized(pipeline_flag, visited_elements,ls_map,pipe_map,barrier,ofile);
	  }
	}


      // write the source side expressions and their 
      // dependencies..
      if(!this->_source->Is_Constant())
	{
	  ofile << "// Source expression" << endl;
	  this->_source->Write_VC_Control_Path_Optimized(pipeline_flag,
							 visited_elements,
							 ls_map,pipe_map, barrier,
							 ofile);
	}

	  
      

      ofile << "// Target expression" << endl;
      this->_target->Write_VC_Control_Path_As_Target_Optimized(pipeline_flag,
							       visited_elements,
							       ls_map,pipe_map,barrier,
							       ofile);




	// four cases.

      // if both are implicit, then declare an interlock.
      bool source_is_implicit = _source->Is_Implicit_Variable_Reference();
      bool target_is_implicit = _target->Is_Implicit_Variable_Reference();

      if(source_is_implicit && target_is_implicit)
	// both are implicit.. introduce an interlock.
	{
	  ofile << "// Interlock " << endl;
	  __DeclTransSplitProtocolPattern

	  ofile <<  ";;[" << this->Get_VC_Name() << "_Sample] { " << endl;
	  ofile << "$T [req] $T [ack] // interlock-sample." << endl;
	  ofile << "}" << endl;

	  ofile <<  ";;[" << this->Get_VC_Name() << "_Update] { " << endl;
	  ofile << "$T [req] $T [ack] // interlock-sample." << endl;
	  ofile << "}" << endl;


	  __ConnectSplitProtocolPattern;

	  if(this->_guard_expression)
	  {
		  if(!this->_guard_expression->Is_Constant())
		  {
			  ofile << "// Guard dependency" << endl;
			  __J(__SST(this), __UCT(this->_guard_expression));
			  if(pipeline_flag)
			  {
				  this->_guard_expression->Write_VC_Update_Reenables(__SCT(this), false,
						  visited_elements, ofile);
			  }
		  }

	  }


	  __J(__SST(this), __UCT(this->_source));
	  if(pipeline_flag)
	  {
		  this->_source->Write_VC_Update_Reenables(__SCT(this), false,
				  visited_elements, ofile);
		  __SelfReleaseSplitProtocolPattern
	  }
	}

      // WAR dependencies
      this->Write_VC_WAR_Dependencies(pipeline_flag, visited_elements,ofile);

      if(!target_is_implicit && !this->_source->Is_Constant())
      {
	      __J(__SST(this->_target), __UCT(this->_source));
	      if(pipeline_flag)
	      {
		      this->_source->Write_VC_Update_Reenables(__SCT(this->_target), false,
				      visited_elements, ofile);
	      }
      }

      if(target_is_implicit)
      {
	      AaRoot* root_obj = _target->Get_Root_Object();
	      if(root_obj == ((AaRoot*) this))
		      visited_elements.insert(this);
	      else if ((root_obj != NULL) && root_obj->Is_Interface_Object())
	      {
		      visited_elements.insert(this);
		      visited_elements.insert(root_obj);
	      }
      }


      this->Write_VC_Synch_Dependency(visited_elements, pipeline_flag, ofile);
      visited_elements.insert(this);
	}
}  


// note: hier-id is already assumed to have been set-up
void AaAssignmentStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
	if(!this->Is_Constant())
	{
		ofile << "// " << this->To_String() << endl;
		ofile << "// " << this->Get_Source_Info() << endl;

		this->_source->Write_VC_Links_Optimized(hier_id,
				ofile);
		this->_target->Write_VC_Links_As_Target_Optimized(hier_id,
				ofile);
		bool source_is_implicit = _source->Is_Implicit_Variable_Reference();
		bool target_is_implicit = _target->Is_Implicit_Variable_Reference();

		if(source_is_implicit && target_is_implicit)
		{
			vector<string> reqs;
			vector<string> acks;
			reqs.push_back(hier_id + "/" + this->Get_VC_Name() + "_Sample/req");
			acks.push_back(hier_id + "/" + this->Get_VC_Name() + "_Sample/ack");
			reqs.push_back(hier_id + "/" + this->Get_VC_Name() + "_Update/req");
			acks.push_back(hier_id + "/" + this->Get_VC_Name() + "_Update/ack");

			Write_VC_Link(this->_target->Get_VC_Datapath_Instance_Name(),
					reqs, acks, ofile);
			reqs.clear();
			acks.clear();
		}
	}
}


// AaCallStatement
void AaCallStatement::Write_VC_Control_Path_Optimized(bool pipeline_flag, 
		set<AaRoot*>& visited_elements,
		map<string, vector<AaExpression*> >& ls_map,
		map<string, vector<AaExpression*> >& pipe_map,
		AaRoot* barrier,
		ostream& ofile)
{
	ofile << "// " << this->To_String() << endl;
	ofile << "// " << this->Get_Source_Info() << endl;

	__DeclTransSplitProtocolPattern

  // take care of the guard
  if(this->_guard_expression)
    {

	if(!this->_guard_expression->Is_Constant())
	{

		ofile << "// Guard expression" << endl;
      		this->_guard_expression->Write_VC_Control_Path_Optimized(pipeline_flag, visited_elements,ls_map,pipe_map,barrier,ofile);

      		__J(__SST(this),__UCT(this->_guard_expression));

		if(pipeline_flag)
		{
			this->_guard_expression->Write_VC_Update_Reenables(__SCT(this), false,
						visited_elements, ofile);
		}
	}
    }

  // first the input arguments... zipping through.
  for(int idx = 0; idx < _input_args.size(); idx++)
  {
    ofile << "// Call input argument " << idx << endl;
    _input_args[idx]->Write_VC_Control_Path_Optimized(pipeline_flag, visited_elements,ls_map,pipe_map,barrier,ofile);
  }


  for(int idx = 0; idx < _input_args.size(); idx++)
    {

      AaExpression* expr = _input_args[idx];
      if(!expr->Is_Constant())
	{
	  __J(__SST(this), __UCT(expr));

      	  if(pipeline_flag)
	  {
	    // expression evaluation will be reenabled by activation of the
	    // call.
	   expr->Write_VC_Update_Reenables(__SCT(this), false,
						visited_elements, ofile);
	  }
	}

    }

  // the call-trigger will start the call..
  ofile << ";;[" << this->Get_VC_Name() << "_Sample] { " 
	<< "$T [crr] $T [cra] "
	<< "} " << endl;
  // the call will eventually complete.
  ofile << ";;[" << this->Get_VC_Name() << "_Update] { " 
	<< "$T [ccr] $T [cca] "
	<< "} " << endl;

  __ConnectSplitProtocolPattern



  // the output arguments.
  bool non_triv_flag = false;
  for(int idx = 0; idx < _output_args.size(); idx++)
    {
      AaExpression* expr = _output_args[idx];

      ofile << "// Call output argument " << idx << endl;
      expr->Write_VC_Control_Path_As_Target_Optimized(pipeline_flag,
						      visited_elements,ls_map,pipe_map,barrier,ofile);
      if(!expr->Is_Implicit_Variable_Reference())
	{

	  __J(__SST(expr),__UCT(this));

	  // if pipeline-flag, then expression activation must reenable call
	  // statement .
	  if(pipeline_flag)
	    {
	      __MJ(__UST(this), __SCT(expr), true); // bypass
	    }

	  non_triv_flag = true;
	}
      

      AaRoot* root_obj = expr->Get_Root_Object();
      if(root_obj == ((AaRoot*) this))
	visited_elements.insert(this);
      else if ((root_obj != NULL) && (root_obj->Is("AaInterfaceObject")))
	{
	  visited_elements.insert(this);
	  visited_elements.insert(root_obj);
	}
    }

  this->Write_VC_Synch_Dependency(visited_elements, pipeline_flag, ofile);
  visited_elements.insert(this);

  // if pipeline-flag, then re-enable..
  if(pipeline_flag)
    {
  	__SelfReleaseSplitProtocolPattern
    }
}


void AaCallStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  ofile << "// " << this->To_String() << endl;
  ofile << "// " << this->Get_Source_Info() << endl;

  for(int idx = 0; idx < _input_args.size(); idx++)
    _input_args[idx]->Write_VC_Links_Optimized(hier_id,ofile);

  for(int idx = 0; idx < _output_args.size(); idx++)
    _output_args[idx]->Write_VC_Links_As_Target_Optimized(hier_id,ofile);

  vector<string> reqs;
  vector<string> acks;

  string start_hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name() + "_Sample");
  reqs.push_back(start_hier_id + "/crr");
  acks.push_back(start_hier_id + "/cra");


  string complete_hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name() + "_Update");
  reqs.push_back(complete_hier_id + "/ccr");
  acks.push_back(complete_hier_id + "/cca");

  Write_VC_Link(this->Get_VC_Name() + "_call",reqs,acks,ofile);
}

// AaBlockStatement
void AaBlockStatement::Identify_Maximal_Sequences(AaStatementSequence* sseq,
						  vector<AaStatementSequence*>& linear_segment_vector)
{
  
  
  // take the statement sequence and chop it into maximal sequences
  // which end in non-control flow statements (place/merge/switch/if)
  // statements.
  //
  //   a sequence with a block statement is a single statement.
  //   a sequence with a control-flow statement is a single statement.
  //   a sequence with a pipe-access is a single statement.
  //   a sequence with simple statements can have many statements.
  //
  //
  int start_idx = 0;
  while(start_idx < sseq->Get_Statement_Count())
    {
      AaStatement* stmt = NULL;
      int end_idx = start_idx;
      
      vector<AaStatement*> linear_segment;
      while(end_idx < sseq->Get_Statement_Count())
	{
	  stmt  = sseq->Get_Statement(end_idx);

	  if(stmt->Is("AaNullStatement"))
	    {
	      end_idx++;
	      continue;
	    }

	  if( stmt->Is_Block_Statement()  || 
	     stmt->Is_Control_Flow_Statement() || 
	     (stmt->Is("AaCallStatement") && 
	      !((AaModule*)(((AaCallStatement*)stmt)->Get_Called_Module()))->Has_No_Side_Effects())
	     || stmt->Can_Block(false))
	    {
	      if(linear_segment.size() == 0)
		{
		  linear_segment.push_back(stmt);
		  end_idx++;
		  break;
		}
	      else
		break;
	    }
	  else 
	    {
	      linear_segment.push_back(stmt);
	      end_idx++;
	    }
	}
      
      if(linear_segment.size() > 0)
	{
	  AaStatementSequence* new_seq = new AaStatementSequence(this,linear_segment);
	  linear_segment_vector.push_back(new_seq);
	  linear_segment.clear();
	}
      
      start_idx = end_idx;
    }
}


void AaBlockStatement::Destroy_Maximal_Sequences(vector<AaStatementSequence*>& linear_segment_vector)
{
  for(int idx = 0, fidx = linear_segment_vector.size(); idx < fidx; idx++)
    {
      delete linear_segment_vector[idx];
    }
  linear_segment_vector.clear();
}


// put stmt in a sequence and call the subsequent method.
void AaBlockStatement::Write_VC_Control_Path_Optimized(AaStatement* stmt, ostream& ofile)
{
  vector<AaStatement*> tv;
  tv.push_back(stmt);
  AaStatementSequence* ss = new AaStatementSequence(this,tv);
  string region_name = ss->Get_VC_Name();
  set<AaRoot*> visited_elements;
  map<string, vector<AaExpression*> > load_store_ordering_map;
  map<string, vector<AaExpression*> >  pipe_map;
  AaRoot* tb = NULL;
  ofile << "::[" << region_name << "] {" << endl;

  this->AaBlockStatement::Write_VC_Control_Path_Optimized(false,ss,visited_elements, load_store_ordering_map, pipe_map, tb, ofile);
  this->Write_VC_Load_Store_Dependencies(false, load_store_ordering_map,ofile);
  this->Write_VC_Pipe_Dependencies(false, pipe_map,ofile);

  ofile << "}" << endl;
  delete ss;
}


// sseq consists of linear sequence of simple statements..
// no control-flow, no block statements.
void AaBlockStatement::Write_VC_Control_Path_Optimized(bool pipeline_flag,
						       AaStatementSequence* sseq,
						       set<AaRoot*>& visited_elements,
						       map<string,vector<AaExpression*> >& load_store_ordering_map,
						       map<string,vector<AaExpression*> >& pipe_map,
						       AaRoot*& trailing_barrier,
						       ostream& ofile)
{
  if(sseq->Get_Statement_Count() == 1 && sseq->Get_Statement(0)->Is_Block_Statement())
    sseq->Get_Statement(0)->Write_VC_Control_Path_Optimized(ofile);
  else
    {
      /////////////////////////////////////////////////  CORE FUNCTIONALITY /////////////////////////////////////////////////////
      trailing_barrier = NULL;
      for(int idx = 0, fidx = sseq->Get_Statement_Count(); idx < fidx; idx++)
	{
	  AaStatement* stmt = sseq->Get_Statement(idx);

	  if(stmt->Is_Block_Statement())
	    {
	      AaRoot::Error("block statement in printing fork block.\n", stmt);
	    }
	  else if(stmt->Is_Control_Flow_Statement())
	    {
	      AaRoot::Error("control-flow statement in printing fork block.\n", stmt);
	    }
	  else
	    {
	      stmt->Write_VC_Control_Path_Optimized(pipeline_flag, 
						    visited_elements,
						    load_store_ordering_map,
						    pipe_map,
						    trailing_barrier,
						    ofile);
	    }

	  if(stmt->Is_Block_Statement() || (stmt->Is("AaCallStatement") && 
					    !((AaModule*)(((AaCallStatement*)stmt)->Get_Called_Module()))->Has_No_Side_Effects())
	     || stmt->Can_Block(pipeline_flag))
	    {
	      trailing_barrier = stmt;
	      ofile << "// barrier: " << stmt->To_String() << endl;

	      // put dependencies from all prior statements 
	      // to the barrier
	      for(int K = idx-1; K >= 0; K--)
		{
		  AaStatement* prev_stmt = sseq->Get_Statement(K);
			
		  // if prev_stmt is a constant, then skip it.
		  if(prev_stmt->Is_Constant())
		    continue;

		  __J(__SST(stmt), __UCT(prev_stmt));
		  if(prev_stmt->Is_Block_Statement() || (prev_stmt->Is("AaCallStatement") && 
							 !((AaModule*)(((AaCallStatement*)prev_stmt)->Get_Called_Module()))->Has_No_Side_Effects())
		     || prev_stmt->Can_Block(pipeline_flag))
		    {
		      break;
		    }
		}
	    }
	}
    }
}



// load store dependencies.
void AaBlockStatement::
Write_VC_Load_Store_Dependencies(bool pipeline_flag, map<string,vector<AaExpression*> >& load_store_dep_map,
				 ostream& ofile)
{

  bool leading_store_found = false;
  set<AaExpression*> leading_accesses;

  set<AaExpression*> trailing_accesses;
  ofile << "// load-store dependencies.." << endl;
  // for each memory space, scan the ordered list of 
  // load-stores, and add dependencies as follows
  //    - start->start dependencies between every load
  //      and nearest store before and after the load
  //    - start->start dependencies between adjacent
  //      stores.
  for(map<string,vector<AaExpression*> >::iterator iter = load_store_dep_map.begin(), 
	fiter  = load_store_dep_map.end();
      iter != fiter;
      iter++)
    {
      vector<AaExpression*> active_loads;
      AaExpression* last_store = NULL;

      string mem_space_name = (*iter).first;

      for(int idx = 0, fidx = (*iter).second.size(); idx < fidx; idx++)
	{
	  AaExpression* expr = (*iter).second[idx];


 	  if(pipeline_flag)
	  {
	  	// keep track of the leading and trailing accesses in
	  	// the sequence..  In the pipeline case, the trailing
	  	// access set will reenable the leading accesses..
		// the scheme: scan the list, and create a leading
		// set of loads (or a singleton store).  At the same
		// time create a trailing set of loads (or a singleton
		// store).  It is possible for the leading and trailing
		// sets to be the same.
	  	if(expr->Is_Store())
	  	{
	      		if(!leading_store_found)
			{
				if(leading_accesses.size() == 0)
					leading_accesses.insert(expr);

				leading_store_found = true;	
			}
	
			trailing_accesses.clear();
			trailing_accesses.insert(expr);
	  	}
	  	else
	  	{
			if(!leading_store_found)
				leading_accesses.insert(expr);

			trailing_accesses.insert(expr);
	  	}
	}

	  if(expr->Is_Store())
	    {


	      if(active_loads.size() > 0)
		{
		  // dependency between active loads and
		  // expr start.
		  for(int lsi = 0, flsi = active_loads.size(); lsi < flsi; lsi++)
		    {
		      Write_VC_Load_Store_Dependency(pipeline_flag, active_loads[lsi],expr,ofile);
		    }

		  // active load dependencies are taken care of
		  active_loads.clear();
		  last_store = expr;
		}
	      else
		{
		  if(last_store != NULL)
		    Write_VC_Load_Store_Dependency(pipeline_flag, last_store,expr,ofile);

		  last_store = expr;
		}
	    }
	  else if(last_store != NULL && expr->Is_Load())
	    {
	      // dependency between last store and expr.
	      Write_VC_Load_Store_Dependency(pipeline_flag, last_store,expr,ofile);
	      
	      // keep track of active loads.
	      active_loads.push_back(expr);
	    }
	  else if(last_store == NULL && expr->Is_Load())
	    {
	      active_loads.push_back(expr);
	    }
	}

	// TODO: in the pipeline case, the last store in the list must
	//       start before the first load/store in the list.  That is,
	//       the ring dependency must be enforced.  Other dependencies
	//       are superfluous!
	if(pipeline_flag)
	{
	      	Write_VC_Load_Store_Loop_Pipeline_Ring_Dependency(mem_space_name,
									leading_accesses,
									trailing_accesses,
									ofile);
      		leading_store_found = false;
      		leading_accesses.clear();
      		trailing_accesses.clear();
	}
    }
}

// pipe accesses will be strictly in order!
void AaBlockStatement::
Write_VC_Pipe_Dependencies(bool pipeline_flag, map<string,vector<AaExpression*> >& pipe_map,
			   ostream& ofile)
{


  for(map<string,vector<AaExpression*> >::iterator iter = pipe_map.begin(), 
	fiter  = pipe_map.end();
      iter != fiter;
      iter++)
    {
      
      ofile << "// pipe read/write dependencies for pipe " << (*iter).first << endl;
      AaExpression* last_expr = NULL;
      AaExpression* first_expr = (*iter).second[0];
      for(int idx = 0, fidx = (*iter).second.size(); idx < fidx; idx++)
	{
	  AaExpression* expr = (*iter).second[idx];
	  if(last_expr != NULL)
	    Write_VC_Pipe_Dependency(pipeline_flag, last_expr,expr,false,ofile );
	  last_expr = expr;
	}
      if(pipeline_flag)
      {
	ofile << "// ring dependency in pipeline." << endl;
	// if last-expression is 
	if((first_expr != NULL) && (last_expr != NULL) && (first_expr != last_expr))
	    Write_VC_Pipe_Dependency(pipeline_flag, last_expr,first_expr,true, ofile);
		// __MJ(__SST(first_expr), __UCT(last_expr), false); // no bypass.
      }
    }
}

void AaBlockStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name());
  if(this->_statement_sequence != NULL)
    this->Write_VC_Links_Optimized(hier_id,this->_statement_sequence, ofile);
}

void AaBlockStatement::Write_VC_Links_Optimized(string hier_id, AaStatement* stmt, ostream& ofile)
{
  vector<AaStatement*> tv;
  tv.push_back(stmt);
  AaStatementSequence* ss = new AaStatementSequence(this,tv);
  this->AaBlockStatement::Write_VC_Links_Optimized(hier_id,ss,ofile);
  delete ss;
}

void AaBlockStatement::Write_VC_Links_Optimized(string hier_id, AaStatementSequence* sseq, ostream& ofile)
{
  if(sseq->Get_Statement_Count() == 1 && sseq->Get_Statement(0)->Is_Block_Statement())
    {
      sseq->Get_Statement(0)->Write_VC_Links_Optimized(hier_id,ofile);
    }
  else
    {
      hier_id = Augment_Hier_Id(hier_id,sseq->Get_VC_Name());
      for(int idx = 0, fidx = sseq->Get_Statement_Count(); idx < fidx; idx++)
	{
	  sseq->Get_Statement(idx)->Write_VC_Links_Optimized(hier_id,ofile);
	}
    }
}


// AaSeriesBlockStatement
void AaSeriesBlockStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  ofile << ";;["  << this->Get_VC_Name()  << "] {" << endl;
  this->Write_VC_Control_Path_Optimized_Base(ofile);
  ofile << "}" << endl;
}


void AaSeriesBlockStatement::Write_VC_Control_Path_Optimized_Base(ostream& ofile)
{
  AaStatementSequence* sseq = this->_statement_sequence;
  if(sseq->Get_Statement_Count() > 0)
    {
      vector<AaStatementSequence* > linear_segment_vector;
      this->Identify_Maximal_Sequences(sseq,linear_segment_vector);
      
      for(int idx = 0, fidx = linear_segment_vector.size(); idx < fidx; idx++)
	{
	  AaStatementSequence* curr_seq = linear_segment_vector[idx];
	  string block_name = curr_seq->Get_VC_Name();

	  if(curr_seq->Get_Statement(0)->Is_Block_Statement())
	    curr_seq->Get_Statement(0)->Write_VC_Control_Path_Optimized(ofile);
	  else
	    {
	      // TODO .. rationalize this.
	      set<AaRoot*> visited_elements;
	      map<string, vector<AaExpression*> > load_store_ordering_map;
	      map<string, vector<AaExpression*> >  pipe_map;
	      AaRoot* tb = NULL;
	      
	      string region_name = curr_seq->Get_VC_Name();
	      ofile << "::[" << region_name << "] {" << endl;
	      this->AaBlockStatement::Write_VC_Control_Path_Optimized(false,
								      curr_seq, 
								      visited_elements,
								      load_store_ordering_map,
								      pipe_map,
								      tb,
								      ofile);
	      this->Write_VC_Load_Store_Dependencies(false, load_store_ordering_map,ofile);
	      this->Write_VC_Pipe_Dependencies(false, pipe_map,ofile);
	      ofile  << "}" << endl;
	    }
	}
      this->Destroy_Maximal_Sequences(linear_segment_vector);
    }
}


void AaSeriesBlockStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name());
  this->Write_VC_Links_Optimized_Base(hier_id,ofile);
}

void AaSeriesBlockStatement::Write_VC_Links_Optimized_Base(string hier_id, ostream& ofile)
{
  AaStatementSequence* sseq = this->_statement_sequence;
  if(sseq->Get_Statement_Count() > 0)
    {
      vector<AaStatementSequence* > linear_segment_vector;
      this->Identify_Maximal_Sequences(sseq,linear_segment_vector);
      
      for(int idx = 0, fidx = linear_segment_vector.size(); idx < fidx; idx++)
	{
	  AaStatementSequence* curr_seq = linear_segment_vector[idx];
	  if(curr_seq->Get_Statement(0)->Is_Block_Statement())
	    curr_seq->Get_Statement(0)->Write_VC_Links_Optimized(hier_id,ofile);
	  else
	    this->AaBlockStatement::Write_VC_Links_Optimized(hier_id,
							     curr_seq,
							     ofile);
	}
      this->Destroy_Maximal_Sequences(linear_segment_vector);
    }  
}

// parallel block statement
void AaParallelBlockStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  ofile << "||["  << this->Get_VC_Name()  << "] {" << endl;
  AaStatementSequence* sseq = this->_statement_sequence;
  for(int idx = 0; idx < sseq->Get_Statement_Count(); idx++)
    {
      AaStatement* stmt = sseq->Get_Statement(idx);
      if(stmt->Is_Block_Statement())
	      stmt->Write_VC_Control_Path_Optimized(ofile);
      else
	      this->AaBlockStatement::Write_VC_Control_Path_Optimized(stmt,ofile);
    }
  ofile << "}" << endl;
}


void AaParallelBlockStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name());
  AaStatementSequence* sseq = this->_statement_sequence;
  for(int idx = 0; idx < sseq->Get_Statement_Count(); idx++)
    {
      AaStatement* stmt = sseq->Get_Statement(idx);
      if(stmt->Is_Block_Statement())
      {
	      stmt->Write_VC_Links_Optimized(hier_id,ofile);
      }
      else
      {
	      this->AaBlockStatement::Write_VC_Links_Optimized(hier_id,stmt,ofile);
      }
    }
}



// AaForkBlockStatement
void AaForkBlockStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  ofile << "// control-path for fork-block " << this->Get_Hierarchical_Name() << endl;
  ofile << "// " << this->Get_Source_Info() << endl;


  ofile << "::[" << this->Get_VC_Name() << "] // fork block " << this->Get_Source_Info() << endl << "{";
  // two passes: first print the statements in this
  // block which are NOT fork/joins.
  if(this->_statement_sequence)
    {
      for(int idx = 0; idx  < this->_statement_sequence->Get_Statement_Count(); idx++)
	{
	  AaStatement* stmt = this->_statement_sequence->Get_Statement(idx);
	  if(!stmt->Is("AaJoinForkStatement"))
	    {
	      vector<AaStatement*> tv;

	      if(stmt->Is_Block_Statement())
		stmt->Write_VC_Control_Path_Optimized(ofile);
	      else
		{
		  this->AaBlockStatement::Write_VC_Control_Path_Optimized(stmt,ofile);
		}


	    }
	}
      
      // second pass, print the fork/joins.
      for(int idx = 0; idx  < this->_statement_sequence->Get_Statement_Count(); idx++)
	{
	  AaStatement* stmt = this->_statement_sequence->Get_Statement(idx);
	  if(stmt->Is("AaJoinForkStatement"))
	    stmt->Write_VC_Control_Path_Optimized(ofile);
	}
    }
  else
    {
      ofile << ";;[DummySB] { $T [dummy] } " << endl;
      ofile << "$entry &-> DummySB" << endl;
      ofile << "$exit <-& DummySB" << endl;
    }
  ofile << "} // end fork block " << this->Get_Source_Info() << endl;
}


void AaForkBlockStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  ofile << "// cp-dp links for fork-block " << this->Get_Hierarchical_Name() << endl;
  ofile << "// " << this->Get_Source_Info() << endl;

  if(this->_statement_sequence)
    {
      hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name());
      for(int idx = 0; idx  < this->_statement_sequence->Get_Statement_Count(); idx++)
	{
	  AaStatement* stmt = this->_statement_sequence->Get_Statement(idx);
	  if(!stmt->Is("AaJoinForkStatement"))
	    {


	      if(stmt->Is_Block_Statement())
		{
		  stmt->Write_VC_Links_Optimized(hier_id,ofile);
		}
	      else
		{
		  this->AaBlockStatement::Write_VC_Links_Optimized(hier_id,stmt,ofile);
		}


	    }
	}
      
      // second pass, print the fork/joins.
      for(int idx = 0; idx  < this->_statement_sequence->Get_Statement_Count(); idx++)
	{
	  AaStatement* stmt = this->_statement_sequence->Get_Statement(idx);
	  if(stmt->Is("AaJoinForkStatement"))
	    stmt->Write_VC_Links_Optimized(hier_id,ofile);
	}
    }
}

// AaBranchBlockStatement
void AaBranchBlockStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  ofile << "<>[" << this->Get_VC_Name() << "] // Branch Block " 
	<< this->Get_Source_Info() << endl << " {" << endl;
  ofile << "$P [" << this->Get_VC_Name() << "__entry__]" << endl;
  ofile << this->Get_VC_Name() << "__entry__ <-| ($entry)" << endl;
  ofile << "$P [" << this->Get_VC_Name() << "__exit__]" << endl;
  ofile << this->Get_VC_Name() << "__exit__ |-> ($exit)" << endl;
  
  this->Write_VC_Control_Path_Optimized(this->Get_VC_Name() + "__entry__", 
					this->_statement_sequence, 
					this->Get_VC_Name() + "__exit__",
					ofile);

  ofile << "} " << endl;
}


void AaBranchBlockStatement::Write_VC_Control_Path_Optimized(string source_link,
							     AaStatementSequence* sseq,
							     string sink_link,
							     ostream& ofile)
{
  bool dummy_flag = true;
  if(sseq->Get_Statement_Count() > 0)
    {
      vector<AaStatementSequence* > linear_segment_vector;
      this->Identify_Maximal_Sequences(sseq,linear_segment_vector);

      if(linear_segment_vector.size() > 0)
	{
	  dummy_flag = false;
	  
	  // first write out the places..
	  for(int idx = 0, fidx = linear_segment_vector.size();  idx < fidx; idx++)
	    {
	      AaStatementSequence* sseq = linear_segment_vector[idx];
	      AaStatement* stmt = sseq->Get_Statement(0);
	      AaStatement* prev = NULL;
	      if(idx > 0)
		{
		  prev = linear_segment_vector[idx-1]->Get_Statement(0);
		}
	      
	      if(!stmt->Is("AaPlaceStatement"))
		{
		  if(!stmt->Is("AaMergeStatement"))
		    ofile << "$P [" << sseq->Get_VC_Entry_Place_Name() << "] " << endl;
		  else
		    {
		      if(prev == NULL || !prev->Is("AaPlaceStatement"))
			{
			  
			  AaMergeStatement* ms = ((AaMergeStatement*)stmt);
			  if(ms->Has_Merge_Label("$entry"))
			    {
			      if(prev != NULL && prev->Is("AaPlaceStatement"))
				{
				  AaRoot::Error("merge statement specifies a merge from entry which is unreachable.",
						stmt);
				}
			    }
			  ms->Set_Has_Entry_Place(true);
			  ofile << "$P [" << stmt->Get_VC_Entry_Place_Name() << "] " << endl;
			}
		    }
		  ofile << "$P [" << sseq->Get_VC_Exit_Place_Name() << "] " << endl;	  
		}
	      else
		stmt->Write_VC_Control_Path_Optimized(ofile);
	    }
	  
	  // second pass, all except merges.
	  for(int idx = 0, fidx = linear_segment_vector.size();  idx < fidx; idx++)
	    {
	      AaStatementSequence* sseq = linear_segment_vector[idx];
	      AaStatement* stmt = sseq->Get_Statement(0);
	      
	      if(!stmt->Is("AaMergeStatement") && !stmt->Is("AaPlaceStatement"))
		{
		  if(stmt->Is_Block_Statement() || stmt->Is_Control_Flow_Statement())
		    stmt->Write_VC_Control_Path_Optimized(ofile);
		  else 
		    {
		      set<AaRoot*> visited_elements;
		      map<string, vector<AaExpression*> > load_store_ordering_map;
		      map<string, vector<AaExpression*> >  pipe_map;
		      AaRoot* tb = NULL;

		      string region_name = sseq->Get_VC_Name();

		      ofile << "::[" << region_name << "] {" << endl;
		      this->AaBlockStatement::Write_VC_Control_Path_Optimized(false, 
									      sseq, 
									      visited_elements,
									      load_store_ordering_map,
									      pipe_map,
									      tb,
									      ofile);

		      this->Write_VC_Load_Store_Dependencies(false, load_store_ordering_map,ofile);
		      this->Write_VC_Pipe_Dependencies(false, pipe_map,ofile);
		      ofile  << "}" << endl;
		    }
		  
		  // control regulated by __entry__ and __exit__ places..
		  // except for switch and if statements..
		  if(!stmt->Is("AaSwitchStatement") && !stmt->Is("AaIfStatement"))
		    {
		      // for switch and if, the control flow is a bit more complicated..
		      // we do not create an explicit CP region for these statements..
		      ofile << sseq->Get_VC_Entry_Place_Name() << " |-> (" << sseq->Get_VC_Name() << ")" << endl;
		      ofile << sseq->Get_VC_Exit_Place_Name() << " <-| (" << sseq->Get_VC_Name() << ")" << endl;
		    }
		}
	    }
	  
	  // third pass, the merges.
	  for(int idx = 0, fidx = linear_segment_vector.size();  idx < fidx; idx++)
	    {
	      AaStatementSequence* sseq = linear_segment_vector[idx];
	      AaStatement* stmt = sseq->Get_Statement(0);
	      
	      if(stmt->Is("AaMergeStatement"))
		{
		  stmt->Write_VC_Control_Path(ofile);	  
		}
	    }
	  
	  
	  // finally, control transfer.
	  for(int idx = 0, fidx = linear_segment_vector.size();  idx < fidx; idx++)
	    {
	      AaStatementSequence* sseq = linear_segment_vector[idx];
	      AaStatement* stmt = sseq->Get_Statement(0);
	      
	      AaStatementSequence* prev = NULL;
	      AaStatementSequence* next = NULL;
	      
	      if(idx > 0)
		prev = linear_segment_vector[idx-1];
	      if(idx < fidx-1)
		{
		  next = linear_segment_vector[idx+1];
		}
	      
	      // stmt gets control from its predecessor..
	      AaStatement* prev_stmt = (prev != NULL ? prev->Get_Statement(0) : NULL);
	      AaStatement* next_stmt = (next != NULL ? next->Get_Statement(0) : NULL);
	      if(prev_stmt == NULL)
		{
		  ofile << sseq->Get_VC_Entry_Place_Name() << " <-| (" << source_link << ")" << endl;
		}
	      else
		{
		  if(!prev_stmt->Is("AaPlaceStatement"))
		    {
		      ofile << sseq->Get_VC_Entry_Place_Name() 
			    << " <-| (" 
			    << prev->Get_VC_Exit_Place_Name() 
			    << ")" << endl;
		    }
		}
	      
	      if(next_stmt == NULL)
		{
		  if(!stmt->Is("AaPlaceStatement"))
		    {
		      ofile << sseq->Get_VC_Exit_Place_Name() << " |-> (" << sink_link << ")" << endl;
		    }
		  
		}
	    } 
	  
	  // clean up..
	  this->Destroy_Maximal_Sequences(linear_segment_vector);
	}      
    }

  if(dummy_flag)
    {
      string plname = source_link + "_to_" + sink_link;
      __Place(plname);
      ofile << plname << " <-| (" << source_link << ")" << endl;
      ofile << plname << " |-> (" << sink_link << ")" << endl;
    }
}


void AaBranchBlockStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  hier_id = Augment_Hier_Id(hier_id,this->Get_VC_Name());
  this->Write_VC_Links_Optimized(hier_id, _statement_sequence, ofile);
}

void AaBranchBlockStatement::Write_VC_Links_Optimized(string hier_id,
						      AaStatementSequence* sseq_in,
						      ostream& ofile)
{
  if(sseq_in->Get_Statement_Count() > 0)
    {

      vector<AaStatementSequence* > linear_segment_vector;
      this->Identify_Maximal_Sequences(sseq_in,linear_segment_vector);

      for(int idx = 0, fidx = linear_segment_vector.size();  idx < fidx; idx++)
	{
	  AaStatementSequence* sseq = linear_segment_vector[idx];
	  AaStatement* stmt = sseq->Get_Statement(0);
	  
	  if(!stmt->Is("AaPlaceStatement"))
	    {
	      if(stmt->Is_Block_Statement() || stmt->Is_Control_Flow_Statement())
		stmt->Write_VC_Links_Optimized(hier_id, ofile);
	      else 
		this->AaBlockStatement::Write_VC_Links_Optimized(hier_id,sseq,ofile);
	    }

	}      

      this->Destroy_Maximal_Sequences(linear_segment_vector);
    }
}


// AaJoinForkStatement
void AaJoinForkStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  ofile << "// control-path for join-fork statement " <<  endl;
  ofile << "// " << this->Get_Source_Info() << endl;
  if(_statement_sequence != NULL)
    {
      for(int idx = 0, fidx = _statement_sequence->Get_Statement_Count();
	  idx < fidx;
	  idx++)
	{
	  AaStatement* stmt = this->_statement_sequence->Get_Statement(idx);
	  if(stmt->Is_Block_Statement())
	    stmt->Write_VC_Control_Path_Optimized(ofile);
	  else
	    {
	      this->AaBlockStatement::Write_VC_Control_Path_Optimized(stmt,ofile);
	    }
	}
    }


  ofile << "$T [" << this->Get_VC_Name() <<"] // join " << this->Get_Source_Info() << endl;
  if(_join_labels.size() == 0)
    {
      ofile << this->Get_VC_Name() << " <-& ($entry)" <<  endl;
    }
  else
    {
      ofile << this->Get_VC_Name() << " <-& (";
      for(int idx = 0; idx < _wait_on_statements.size(); idx++)
	{
	  if(idx > 0)
	    ofile << " ";
	  ofile << _wait_on_statements[idx]->Get_VC_Name();
	}
      ofile << ")" << endl;
    }

  if(_statement_sequence == NULL)
    {
      ofile << this->Get_VC_Name() << " &-> ($exit)" <<  endl;
    }
  else
    {
      ofile << this->Get_VC_Name() << " &-> (";
      for(int idx = 0; idx < _statement_sequence->Get_Statement_Count(); idx++)
	{
	  if(idx > 0)
	    ofile << " ";
	  ofile << _statement_sequence->Get_Statement(idx)->Get_VC_Name();
	}
      ofile << ")" << endl;
    }
}
void AaJoinForkStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  ofile << "// CP-DP links for join-fork  " << endl;
  ofile << "// " << this->Get_Source_Info() << endl;

  if(_statement_sequence != NULL)
    {
      for(int idx = 0, fidx = _statement_sequence->Get_Statement_Count();
	  idx < fidx;
	  idx++)
{
	  AaStatement* stmt = this->_statement_sequence->Get_Statement(idx);
	  if(stmt->Is_Block_Statement())
	    stmt->Write_VC_Links_Optimized(hier_id,ofile);
	  else
	    {
	      this->AaBlockStatement::Write_VC_Links_Optimized(hier_id, stmt, ofile);
	    }
	}
    }
}


// AaMergeStatement
void AaMergeStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  this->Write_VC_Control_Path(ofile);
}
void AaMergeStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  this->Write_VC_Links(hier_id,ofile);
}

// AaPhiStatement
void AaPhiStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  this->Write_VC_Control_Path(ofile);
}


// implemented for supporting loop pipelining.  This statement should
// be called only if the PHI statement appears in a loop body.
//
//    The control flow instantiates a phi-sequencer and keys off
// the place transitions associated with the loop block body
// in which the statement occurs.
void AaPhiStatement::Write_VC_Control_Path_Optimized(bool pipeline_flag,
						     set<AaRoot*>& visited_elements,
						     map<string, vector<AaExpression*> >& ls_map,
						     map<string,vector<AaExpression*> >& pipe_map,
						     AaRoot* barrier,
						     ostream& ofile)
{
  bool ok_flag = this->Get_In_Do_While();
  if(!ok_flag)
    assert(0);

  assert(pipeline_flag);

  ofile << "// (pipelined) PHI statement " << this->Get_VC_Name() << endl;
  ofile << "// " << this->To_String() << endl;

  // the active, completed and the active transitions
  string trigger_from_loop_back = this->Get_VC_Name() + "_loopback_trigger";
  string sample_from_loop_back_from_phi_seq = this->Get_VC_Name() + "_loopback_sample_req_from_phi_seq";
  string sample_from_loop_back = this->Get_VC_Name() + "_loopback_sample_req";
  __T(trigger_from_loop_back);
  __T(sample_from_loop_back_from_phi_seq);
  __T(sample_from_loop_back);
  __J(trigger_from_loop_back,"back_edge_to_loop_body");
  __J(sample_from_loop_back, sample_from_loop_back_from_phi_seq);

  string trigger_from_entry = this->Get_VC_Name() + "_entry_trigger";
  string sample_from_entry = this->Get_VC_Name() + "_entry_sample_req";
  string sample_from_entry_from_phi_seq = this->Get_VC_Name() + "_entry_sample_req_from_phi_seq";
  __T(trigger_from_entry);
  __T(sample_from_entry_from_phi_seq);
  __T(sample_from_entry);
  __J(trigger_from_entry, "first_time_through_loop_body");
  __J(sample_from_entry, sample_from_entry_from_phi_seq);


  __DeclTransSplitProtocolPattern;

  if(pipeline_flag)
  {
	__MJ(__UST(this), __UCT(this), true); // bypass.
	__MJ(__SST(this), __SCT(this), false); // no bypass.
  }

  string msample_req = this->Get_VC_Name() + "_merged_reqs";
  string req_merge_name = this->Get_VC_Name() + "_req_merge";
  __T(msample_req);

  ofile << "$transitionmerge [" << req_merge_name << "] (" 
	<< sample_from_entry << " " << sample_from_loop_back << ") (" << msample_req << ")" << endl;
  __F(msample_req, "$null"); // merged the two and tied the merge as open.

  string phi_triggers;
  string phi_reqs;
  // the source control paths.
  for(int idx = 0, fidx = _source_pairs.size(); idx < fidx; idx++)
    {
      AaExpression* source_expr = _source_pairs[idx].second;
      string trig_place = _source_pairs[idx].first;

      if(!source_expr->Is_Constant())
	{
	  source_expr->Write_VC_Control_Path_Optimized(pipeline_flag,
						       visited_elements,
						       ls_map,pipe_map,barrier,
						       ofile);
	  __J(__SST(this),__UCT(source_expr));
	  if(pipeline_flag)
	  {
		source_expr->Write_VC_Update_Reenables(__SCT(this), false,
						visited_elements, ofile);
	  }
        }
      if(trig_place == "$loopback")
      {
	      phi_triggers += trigger_from_loop_back  + " "; 
	      phi_reqs += sample_from_loop_back_from_phi_seq + " ";
      }
      else
      {
	      phi_triggers +=  trigger_from_entry + " ";
	      phi_reqs += sample_from_entry_from_phi_seq + " ";
      }
    }

  string  phi_sequencer_done = this->Get_VC_Name() + "_phi_sequencer_done";
  __T(phi_sequencer_done);
  __F(phi_sequencer_done, "$null");


  // the control-sequencing for the PHI is too complicated to
  // model in a normal-fork block. The dirty stuff is hidden
  // in the phi_sequencer.
  ofile << "$phisequencer [ " << this->Get_VC_Name() << "_phi_seq] ( ";
  ofile << phi_triggers << " : ";
  ofile << __SST(this) << " : " ;
  ofile << __SCT(this) << " ) ( " ;
  ofile << phi_reqs << " : " << phi_sequencer_done << " ) " << endl;

  // sample-ack from phi must join with condition-evaluated so that
  // loop-back is delayed until the present PHI has sampled.
  __J("condition_evaluated", __SCT(this));

  // take care of the guard
  if(this->_guard_expression)
    {
		// For the moment, PHI statements cannot have guards.
	    AaRoot::Error("Guards for PHI statements are not permitted.", this);
    }
  visited_elements.insert(this);
}

// called only if it appears in a do-while loop.
void AaPhiStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  bool ok_flag = this->Get_In_Do_While();
  if(!ok_flag)
    assert(0);

  vector<string> reqs;
  vector<string> acks;

  string sample_from_loop_back = this->Get_VC_Name() + "_loopback_sample_req";
  string sample_from_entry = this->Get_VC_Name() + "_entry_sample_req";
  string sample_ack = __SCT(this);
  string update_req = __UST(this);
  string update_ack = __UCT(this);

  for(int idx = 0, fidx = _source_pairs.size(); idx < fidx; idx++)
    {
      string trig_place = _source_pairs[idx].first;
      if(trig_place == "$loopback")
	reqs.push_back(hier_id + "/" + sample_from_loop_back);
      else
	reqs.push_back(hier_id + "/" + sample_from_entry);
    }
  reqs.push_back(hier_id + "/" + update_req);
  acks.push_back(hier_id + "/" + sample_ack);
  acks.push_back(hier_id + "/" + update_ack);
  
  Write_VC_Link(this->Get_VC_Name(),reqs,acks,ofile);

  for(int idx = 0, fidx = _source_pairs.size(); idx < fidx; idx++)
    {
      AaExpression* source_expr = _source_pairs[idx].second;
      source_expr->Write_VC_Links_Optimized(hier_id, ofile);
    }
}

// AaSwitchStatement
void AaSwitchStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  this->Write_VC_Control_Path(true, ofile);
}
void AaSwitchStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  this->Write_VC_Links(true, hier_id, ofile);
}

// AaIfStatement
void AaIfStatement::Write_VC_Control_Path_Optimized(ostream& ofile)
{
  this->Write_VC_Control_Path(true, ofile);  
}

void AaIfStatement::Write_VC_Links_Optimized(string hier_id, ostream& ofile)
{
  this->Write_VC_Links(true, hier_id,ofile);
}
