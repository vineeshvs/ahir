#include <vcIncludes.hpp>
#include <vcRoot.hpp>
#include <vcType.hpp>
#include <vcValue.hpp>
#include <vcControlPath.hpp>
#include <vcOperator.hpp>
#include <vcMemorySpace.hpp>
#include <vcDataPath.hpp>
#include <vcModule.hpp>
#include <vcSystem.hpp>


void vcPipe::Print_VHDL_Pipe_Port_Signals(ostream& ofile)
{
  string pipe_id = To_VHDL(this->Get_Id());
  int pipe_width = this->Get_Width();
      
  int num_reads = this->Get_Pipe_Read_Count();
  int num_writes = this->Get_Pipe_Write_Count();
     
  if(num_reads > 0 && num_writes ==  0)
    {
	    ofile << "-- write to pipe " << pipe_id << endl;
	    ofile << "signal " 
		    << pipe_id 
		    << "_pipe_write_data: std_logic_vector(" << pipe_width-1 << " downto 0);" << endl;
	    ofile << "signal " << pipe_id << "_pipe_write_req : std_logic_vector(0 downto 0) := (others => '0');" << endl;
	    ofile << "signal " << pipe_id << "_pipe_write_ack : std_logic_vector(0 downto 0);" << endl;
	    if(this->Get_Port())
	    {
		    ofile << "signal " 
			    << pipe_id 
			    << ": std_logic_vector(" << (this->Get_Signal() ? 0 : (pipe_width-1)) << " downto 0);" << endl;
	    }
    }

  if(num_writes > 0 && num_reads == 0)
  {
	  ofile << "-- read from pipe " << pipe_id << endl;
		  ofile << "signal "
			  << pipe_id << "_pipe_read_data: std_logic_vector(" << pipe_width-1 << " downto 0);" << endl;
		  ofile << "signal " << pipe_id << "_pipe_read_req : std_logic_vector(0 downto 0) := (others => '0');" << endl;
		  ofile << "signal " << pipe_id << "_pipe_read_ack : std_logic_vector(0 downto 0);" << endl;
	  if(this->Get_Port())
		  ofile << "signal " << pipe_id << ": std_logic_vector(" << (this->Get_Signal() ? 0 : (pipe_width-1)) << " downto 0);" << endl;
  }
}


void vcPipe::Print_VHDL_Pipe_Signals(ostream& ofile)
{
	string pipe_id = this->Get_Id();
	int pipe_width = this->Get_Width();

	int num_reads = this->Get_Pipe_Read_Count();
	int num_writes = this->Get_Pipe_Write_Count();

	if(num_writes >  0)
	{
		ofile << "-- aggregate signals for write to pipe " << pipe_id << endl;
		ofile << "signal " << pipe_id << "_pipe_write_data: std_logic_vector(" << (num_writes*pipe_width)-1 << " downto 0);" << endl;
		ofile << "signal " << pipe_id << "_pipe_write_req: std_logic_vector(" << num_writes-1 << " downto 0);" << endl;
		ofile << "signal " << pipe_id << "_pipe_write_ack: std_logic_vector(" << num_writes-1 << " downto 0);" << endl;
	}

	if(num_reads > 0)
	{
		ofile << "-- aggregate signals for read from pipe " << pipe_id << endl;
		ofile << "signal " << pipe_id << "_pipe_read_data: std_logic_vector(" << (num_reads*pipe_width)-1 << " downto 0);" << endl;
		ofile << "signal " << pipe_id << "_pipe_read_req: std_logic_vector(" << num_reads-1 << " downto 0);" << endl;
		ofile << "signal " << pipe_id << "_pipe_read_ack: std_logic_vector(" << num_reads-1 << " downto 0);" << endl;
	}

}

void vcPipe::Print_VHDL_Instance(ostream& ofile)
{
	string pipe_id = To_VHDL(this->Get_Id());
	int pipe_width = this->Get_Width();
	int pipe_depth = this->Get_Depth();

	int num_reads = this->Get_Pipe_Read_Count();
	int num_writes = this->Get_Pipe_Write_Count();

	if(num_reads > 0 || num_writes > 0)
	{
		// avert impossibility..
		assert(!this->Get_Port() || (this->Get_In_Flag() || this->Get_Out_Flag()));

		// the pipe may be used in one of three modes.
		// - as an in-flag to pipe converter.
		//     (pulse_mode implies that input pulses are held
		//      until sampled by the reader).
		// - as a pipe to out-flag converter
		//     (pulse mode implies that a single output pulse
		//      is created by each successful writer).
		// - as a normal pipe with read and write interfaces.
		if(this->Get_Port() && this->Get_In_Flag())
		{
			assert(num_writes == 0);
			ofile << pipe_id << "_InPort: SystemInPort -- {" << endl;
			ofile << "generic map( -- { " << endl;
			ofile << "name => " << '"' << "pipe " << pipe_id << '"' << "," << endl;
			ofile << "num_reads => " << num_reads << "," << endl;
			ofile << "in_data_width => " << (this->Get_Signal() ? 1 : pipe_width) << "," << endl;
			ofile << "out_data_width => " << pipe_width << " --}\n)" << endl;
			ofile << "port map( -- { " << endl;
			ofile << "read_req => " << pipe_id << "_pipe_read_req," << endl 
				<< "read_ack => " << pipe_id << "_pipe_read_ack," << endl 
				<< "read_data => "<< pipe_id << "_pipe_read_data," << endl 
				<< "in_data => "<< pipe_id << "," << endl 
				<< "clk => clk,"
				<< "reset => reset -- }\n ); -- }" << endl;
		}
		else if(this->Get_Out_Flag() && this->Get_Port())
		{ 
			assert(num_reads == 0);
			ofile << pipe_id << "_OutPort: SystemOutPort -- {" << endl;
			ofile << "generic map( -- { " << endl;
			ofile << "name => " << '"' << "pipe " << pipe_id << '"' << "," << endl;
			ofile << "num_writes => " << num_writes << "," << endl;
			ofile << "out_data_width => " << (this->Get_Signal() ? 1 : pipe_width) << "," << endl;
			ofile << "in_data_width => " << pipe_width << " --} ) \n" << endl;
			ofile << "port map( -- { " << endl;
			ofile << "write_req => " << pipe_id << "_pipe_write_req," << endl 
				<< "write_ack => " << pipe_id << "_pipe_write_ack," << endl 
				<< "write_data => "<< pipe_id << "_pipe_write_data," << endl 
				<< "out_data => "<< pipe_id << "," << endl 
				<< "clk => clk,"
				<< "reset => reset -- }\n ); -- }" << endl;
		}
		else
		{
			num_reads = MAX(num_reads,1);
		  num_writes = MAX(num_writes,1);
		  ofile << pipe_id << "_Pipe: PipeBase -- {" << endl;
		  ofile << "generic map( -- { " << endl;
		  ofile << "name => " << '"' << "pipe " << pipe_id << '"' << "," << endl;
		  ofile << "num_reads => " << num_reads << "," << endl;
		  ofile << "num_writes => " << num_writes << "," << endl;
		  ofile << "data_width => " << pipe_width << "," << endl;
		  ofile << "lifo_mode => " << (this->Get_Lifo_Mode() ? "true" : "false") << "," << endl;
		  ofile << "depth => " << pipe_depth << " --}\n)" << endl;
		  ofile << "port map( -- { " << endl;
		  ofile << "read_req => " << pipe_id << "_pipe_read_req," << endl 
			  << "read_ack => " << pipe_id << "_pipe_read_ack," << endl 
			  << "read_data => "<< pipe_id << "_pipe_read_data," << endl 
			  << "write_req => " << pipe_id << "_pipe_write_req," << endl 
			  << "write_ack => " << pipe_id << "_pipe_write_ack," << endl 
			  << "write_data => "<< pipe_id << "_pipe_write_data," << endl 
			  << "clk => clk,"
			  << "reset => reset -- }\n ); -- }" << endl;
	  }
  }
  else
  {
	  vcSystem::Warning("pipe " + pipe_id + " not used in the system, ignored");
  }
}

bool vcPipe::Get_Pipe_Module_Section(vcModule* caller_module, 
		string read_or_write, 
		int& hindex, 
		int& lindex)
{

	bool ret_val = false;
	hindex = 
		((read_or_write == "read") ? this->Get_Pipe_Read_Count()-1 :
		 this->Get_Pipe_Write_Count() -1);


	map<vcModule*, vector<int> >::iterator iter, fiter;

	if(read_or_write == "read")
	{
		iter = this->_pipe_read_map.begin();
		fiter = this->_pipe_read_map.end();
	}
	else
	{
		iter = this->_pipe_write_map.begin();
		fiter = this->_pipe_write_map.end();
	}

	for(; iter != fiter; iter++ )
	{
		if(caller_module == (*iter).first)
		{
			lindex = (hindex + 1) - (*iter).second.size();
			ret_val = true;
			break;
		}
		else
		{
			hindex -= (*iter).second.size();
		}
	}
	return(ret_val);
}

string vcPipe::Get_Pipe_Aggregate_Section(string pid, 
		int hindex, 
		int lindex) 
{

	int data_width;
	string ret_string = this->Get_VHDL_Pipe_Interface_Port_Name(pid);
    
  // find data_width.
  if((pid.find("req") != string::npos) || (pid.find("ack") != string::npos))
    data_width = 1;
  else if(pid.find("data") != string::npos)
    data_width = this->Get_Width();
  else
    assert(0); // fatal
    
  ret_string += "(";
  ret_string += IntToStr(((hindex+1)*data_width)-1);
  ret_string += " downto ";
  ret_string += IntToStr(lindex*data_width);
  ret_string += ")";
  return(ret_string);
}

vcWire::vcWire(string id, vcType* t) :vcRoot(id)
{
  this->_type = t;
}

void vcWire::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__WIRE] << " " << this->Get_Id() << vcLexerKeywords[__COLON] << " " ;
  this->Get_Type()->Print(ofile);
  ofile << endl;
}

void vcWire::Print_VHDL_Std_Logic_Declaration(ostream& ofile)
{
  ofile << "signal " << this->Get_VHDL_Signal_Id() << " : " << this->Get_Type()->Get_VHDL_Type_Name() << ";" << endl;
}

int vcWire::Get_Size() {return(this->_type->Size());}

vcConstantWire::vcConstantWire(string id, vcValue* v): vcWire(id,v->Get_Type())
{
  assert(!(v->Is("vcArrayType") || v->Is("vcRecordType")));
  this->_value = v;
};

void vcConstantWire::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__CONSTANT] << " ";
  ofile << vcLexerKeywords[__WIRE] << " " << this->Get_Id() << vcLexerKeywords[__COLON] << " " ;
  this->Get_Type()->Print(ofile);
  ofile << vcLexerKeywords[__ASSIGNEQUAL] << " ";
  this->_value->Print(ofile);
  ofile << endl;
}

void vcConstantWire::Print_VHDL_Constant_Declaration(ostream& ofile)
{
  ofile << "constant " << this->Get_VHDL_Signal_Id() << " : " ;
  ofile << this->Get_Type()->Get_VHDL_Type_Name()  << " := ";
  ofile << this->_value->To_VHDL_String() << ";" << endl;
}

vcInputWire::vcInputWire(string id, vcType* t): vcWire(id,t)
{
}

vcOutputWire::vcOutputWire(string id, vcType* t): vcWire(id,t)
{
}
 
bool vcDatapathElement::Is_Part_Of_Pipelined_Loop(int& depth, int& buffering)
{
	bool ret_val = false;
	depth = 1;
	buffering = 1;
	vcCPElement* t = NULL;
	// just check a req or an ack and see if it
	// is part of a pipelined loop.
	if(this->Get_Number_Of_Reqs() > 0)
		t = this->Get_Req(0);
	else if(this->Get_Number_Of_Acks() > 0)
		t = this->Get_Ack(0);
	
	if(t != NULL)
	{
		vcCPPipelinedForkBlock* p  = t->Get_Pipeline_Parent();
		if(p != NULL)
		{
			vcCPElement* pp = p->Get_Parent();
			if(pp->Is("vcCPSimpleLoopBlock"))
			{
				ret_val = true;
				buffering = ((vcCPSimpleLoopBlock*) pp)->Get_Pipeline_Buffering();
				depth = ((vcCPSimpleLoopBlock*) pp)->Get_Pipeline_Depth();
			}
			else if(pp->Is("vcControlPath"))
			{
				ret_val = true;
				buffering = ((vcControlPath*) pp)->Get_Pipeline_Buffering();
				depth = ((vcControlPath*) pp)->Get_Pipeline_Depth();
			}
		}
	}
	return(ret_val);
}

void vcDatapathElement::Print_VHDL_Logger(ostream& ofile)
{
	string id = this->Get_Id();
	for(int idx = 0, fidx = _reqs.size(); idx < fidx; idx++)
	{
		vcTransition* r = _reqs[idx];
		if(r != NULL)
		{
			ofile << "LogCPEvent(clk, reset, global_clock_cycle_count,";
			ofile << r->Get_CP_To_DP_Symbol() << ","
				<< '"' 
				<< " req" << idx << " " << id
				<< '"'
				<< ");" << endl;
		}
	}
	for(int idx = 0, fidx = _acks.size(); idx < fidx; idx++)
	{
		vcTransition* a = _acks[idx];
		if(a != NULL)
		{
			ofile << "LogCPEvent(clk, reset, global_clock_cycle_count,";
			ofile << a->Get_DP_To_CP_Symbol() << ","
				<< '"' 
				<< " ack" << idx << " " << id
				<< '"'
				<< ");" << endl;
		}
	}
}

vcDataPath::vcDataPath(vcModule* m, string id):vcRoot(id)
{
  this->_parent = m;
}

void vcDataPath::Get_Label_Interval(vcControlPath* cp, vcDatapathElement* dpe, vector<vcCompatibilityLabel*>& ret_vector)
{
  int i, i_f;
  for(i = 0, i_f = dpe->Get_Number_Of_Reqs(); i < i_f; i++)
  {
	vcCPElement* r = dpe->Get_Req(i);
	assert(r != NULL);
	     
	ret_vector.push_back(r->Get_Compatibility_Label());
  }
  for(i = 0, i_f = dpe->Get_Number_Of_Acks(); i < i_f; i++)
  {
	vcCPElement* a = dpe->Get_Ack(i);
	assert(a != NULL);

	ret_vector.push_back(a->Get_Compatibility_Label());
  }
}

vcDatapathElement* vcDataPath::Find_DPE(string dpe_name)
{
  map<string, vcDatapathElement*>::iterator iter = this->_dpe_map.find(dpe_name);
  if(iter != this->_dpe_map.end())
    return((*iter).second);
  else
    return(NULL);
}

vcWire* vcDataPath::Find_Wire(string wname)
{
  vcWire* ret_wire = NULL;
  map<string, vcWire*>::iterator iter = this->_wire_map.find(wname);
  if(iter != this->_wire_map.end())
    ret_wire = ((*iter).second);
  else
    {
      if(this->Get_Parent() != NULL)
	{
	  ret_wire = this->Get_Parent()->Get_Argument(wname,"in");
	  if(ret_wire == NULL)
	    ret_wire = this->Get_Parent()->Get_Argument(wname,"out");
	}
    }

  if(ret_wire == NULL)
    {
      // perhaps it is a constant defined at the Program scope
      ret_wire = (vcWire*) this->Get_Parent()->Get_Parent()->Find_Constant_Wire(wname);
    }

  return(ret_wire);
}

void vcDataPath::Add_Wire(string wname, vcType* t)
{
  assert(this->Find_Wire(wname) == NULL);
  this->_wire_map[wname] = new vcWire(wname, t);
}

void vcDataPath::Add_Intermediate_Wire(string wname, vcType* t)
{
  assert(this->Find_Wire(wname) == NULL);
  this->_wire_map[wname] = new vcIntermediateWire(wname, t);
}

void vcDataPath::Add_Constant_Wire(string wname, vcValue* v)
{
  assert(v != NULL);
  vcType* t = v->Get_Type();
  assert(!(t->Is("vcArrayType") || t->Is("vcRecordType")));
  this->_wire_map[wname] = (vcWire*) ( new vcConstantWire(wname,v));
};

#define __PRINT_MAP(_map_id,_map_iter,ofile)  for(_map_iter=_map_id.begin();_map_iter!=_map_id.end();_map_iter++)\
    { (*_map_iter).second->Print(ofile);}

void vcDataPath::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__DATAPATH] << " { " << endl;

  map<string,vcWire*>::iterator wires;
  __PRINT_MAP(_wire_map,wires,ofile);

  map<string,vcPhi*>::iterator phis;
  __PRINT_MAP(_phi_map,phis,ofile);

  map<string,vcSelect*>::iterator selects;
  __PRINT_MAP(_select_map,selects,ofile);

  map<string,vcSlice*>::iterator slices;
  __PRINT_MAP(_slice_map,slices,ofile);

  map<string,vcBranch*>::iterator branches;
  __PRINT_MAP(_branch_map,branches,ofile);

  map<string,vcLoad*>::iterator loads;
  __PRINT_MAP(_load_map,loads,ofile);

  map<string,vcStore*>::iterator stores;
  __PRINT_MAP(_store_map,stores,ofile);

  map<string,vcSplitOperator*>::iterator split_operators;
  __PRINT_MAP(_split_operator_map,split_operators,ofile);

  map<string,vcInport*>::iterator input_ports;
  __PRINT_MAP(_inport_map,input_ports,ofile);

  map<string,vcOutport*>::iterator output_ports;
  __PRINT_MAP(_outport_map,output_ports,ofile);

  this->Print_Attributes(ofile);
  ofile << "} " << endl;
}

#define _ADD(_map_id,_id,_ptr) _map_id[_id]=_ptr;\
  if(_dpe_map.find(_id)!=_dpe_map.end())\
    vcSystem::Error("multiple DPE instances with id " + _id);\
  else\
    _dpe_map[_id]=(vcDatapathElement*)_ptr;

#define __FIND(_map_id,_id) (_map_id.find(_id) != _map_id.end() ? _map_id[_id] : NULL) 

void vcDataPath::Add_Phi(vcPhi* p) {_ADD(_phi_map,p->Get_Id(), p);}
vcPhi* vcDataPath::Find_Phi(string id) {return(__FIND(_phi_map,id));}

void vcDataPath::Add_Load(vcLoad* ld) 
{
  _ADD(_load_map,ld->Get_Id(), ld);
  this->Get_Parent()->Add_Accessed_Memory_Space(ld->Get_Memory_Space());
}
vcLoad* vcDataPath::Find_Load(string id) {return(__FIND(_load_map,id));}

void vcDataPath::Add_Store(vcStore* st) 
{
  _ADD(_store_map,st->Get_Id(), st);
  this->Get_Parent()->Add_Accessed_Memory_Space(st->Get_Memory_Space());
}
vcStore* vcDataPath::Find_Store(string id)  {return(__FIND(_store_map,id));}

void vcDataPath::Add_Call(vcCall* c)  
{
  _ADD(_call_map,c->Get_Id(), c);
  this->Get_Parent()->Add_Called_Module(c->Get_Called_Module());
}
vcCall* vcDataPath::Find_Call(string id) {return(__FIND(_call_map,id));}

void vcDataPath::Add_Inport(vcInport* p) 
{
  _ADD(_inport_map,p->Get_Id(), p);
}
vcInport* vcDataPath::Find_Inport(string id) {return(__FIND(_inport_map,id));}

void vcDataPath::Add_Outport(vcOutport* p) 
{
  _ADD(_outport_map,p->Get_Id(), p);
}
vcOutport* vcDataPath::Find_Outport(string id) {return(__FIND(_outport_map,id));}


void vcDataPath::Add_Split_Operator(vcSplitOperator* p)  {_ADD(_split_operator_map,p->Get_Id(), p);}
vcSplitOperator* vcDataPath::Find_Split_Operator(string id) {return(__FIND(_split_operator_map,id));}

void vcDataPath::Add_Select(vcSelect* p)   {_ADD(_select_map,p->Get_Id(), p);}
vcSelect* vcDataPath::Find_Select(string id) {return(__FIND(_select_map,id));}

void vcDataPath::Add_Slice(vcSlice* p)   {_ADD(_slice_map,p->Get_Id(), p);}
vcSlice* vcDataPath::Find_Slice(string id) {return(__FIND(_slice_map,id));}

void vcDataPath::Add_Permutation(vcPermutation* p)   {_ADD(_permutation_map,p->Get_Id(), p);}
vcPermutation* vcDataPath::Find_Permutation(string id) {return(__FIND(_permutation_map,id));}

void vcDataPath::Add_Branch(vcBranch* p)  {_ADD(_branch_map,p->Get_Id(), p);}
vcBranch* vcDataPath::Find_Branch(string id) {return(__FIND(_branch_map,id));}

void vcDataPath::Add_Register(vcRegister* p)  {_ADD(_register_map,p->Get_Id(), p);}
vcRegister* vcDataPath::Find_Register(string id) {return(__FIND(_register_map,id));}

void vcDataPath::Add_Interlock_Buffer(vcInterlockBuffer* p)  
	{_ADD(_interlock_buffer_map,p->Get_Id(), p);}
vcInterlockBuffer* vcDataPath::Find_Interlock_Buffer(string id) 
	{return(__FIND(_interlock_buffer_map,id));}

void vcDataPath::Add_Equivalence(vcEquivalence* p)  {_ADD(_equivalence_map,p->Get_Id(), p);}
vcEquivalence* vcDataPath::Find_Equivalence(string id) {return(__FIND(_equivalence_map,id));}


void vcDataPath::Compute_Maximal_Groups(vcControlPath* cp)
{
  for(map<string, vcDatapathElement*>::iterator dpe_iter = _dpe_map.begin();
      dpe_iter != _dpe_map.end();
      dpe_iter++)
    {
      if((*dpe_iter).second->Kind() == "vcUnarySplitOperator" ||
	 (*dpe_iter).second->Kind() == "vcBinarySplitOperator")
	{
	  this->Update_Maximal_Groups(cp,(*dpe_iter).second, _compatible_split_operator_groups);
	}
      else if((*dpe_iter).second->Kind() == "vcLoad")
	{
	  this->Update_Maximal_Groups(cp, (*dpe_iter).second, _compatible_load_groups);
	}
      else if((*dpe_iter).second->Kind() == "vcStore")
	{
	  this->Update_Maximal_Groups(cp, (*dpe_iter).second, _compatible_store_groups);
	}
      else if((*dpe_iter).second->Kind() == "vcCall")
	{
	  this->Update_Maximal_Groups(cp, (*dpe_iter).second, _compatible_call_groups);
	}
      else if((*dpe_iter).second->Kind() == "vcOutport")
	{
	  this->Update_Maximal_Groups(cp, (*dpe_iter).second, _compatible_outport_groups);
	}
      else if((*dpe_iter).second->Kind() == "vcInport")
	{
	  this->Update_Maximal_Groups(cp, (*dpe_iter).second, _compatible_inport_groups);
	}
    }

  // load, store and call groups linked with the respective 
  // objects.
  for(int idx = 0; idx < this->_compatible_load_groups.size(); idx++)
    {
      vcMemorySpace* ms = ((vcLoad*) (*(_compatible_load_groups[idx].begin())))->Get_Memory_Space();
      ms->Register_Load_Group(this->_parent, idx, _compatible_load_groups[idx].size());
    }

  for(int idx = 0; idx < this->_compatible_store_groups.size(); idx++)
    {
      vcMemorySpace* ms = ((vcStore*) (*(_compatible_store_groups[idx].begin())))->Get_Memory_Space();
      ms->Register_Store_Group(this->_parent, idx, _compatible_store_groups[idx].size());
    }

  for(int idx = 0; idx < this->_compatible_call_groups.size(); idx++)
    {
      vcModule* cm = ((vcCall*) (*(_compatible_call_groups[idx].begin())))->Get_Called_Module();
      cm->Register_Call_Group(this->_parent, idx, _compatible_call_groups[idx].size());
    }

  for(int idx = 0; idx < this->_compatible_inport_groups.size(); idx++)
    {
      vcPipe* p = ((vcInport*) (*(_compatible_inport_groups[idx].begin())))->Get_Pipe();
      _inport_group_map[p].push_back(idx);
      p->Register_Pipe_Read(this->_parent,idx);
    }

  for(int idx = 0; idx < this->_compatible_outport_groups.size(); idx++)
    {
      vcPipe* p = ((vcOutport*) (*(_compatible_outport_groups[idx].begin())))->Get_Pipe();
      _outport_group_map[p].push_back(idx);
      p->Register_Pipe_Write(this->_parent,idx);
    }

}


void vcDataPath::Update_Maximal_Groups(vcControlPath* cp, 
				       vcDatapathElement* dpe, 
				       vector<set<vcDatapathElement*> >& dpe_group)
{

  bool new_group = true;
  int idx_of_group_to_which_added = -1;

  vector<vcCompatibilityLabel*> labels_1;

  if(!vcSystem::_min_area_flag)
  	this->Get_Label_Interval(cp,dpe,labels_1);

  for(int idx = 0; idx < dpe_group.size(); idx++)
    {
      if(dpe->Is_Shareable_With(*(dpe_group[idx].begin())))
	{
	  bool is_compatible = true;

	  if(!vcSystem::_min_area_flag && !dpe->Is_Pipelined_Operator())
	    {
	      for(set<vcDatapathElement*>::iterator dpe_iter = dpe_group[idx].begin();
		  dpe_iter != dpe_group[idx].end();
		  dpe_iter++)
		{
  		  vector<vcCompatibilityLabel*> labels_2;
		  this->Get_Label_Interval(cp,*dpe_iter, labels_2);

		  if(!cp->Are_Compatible(labels_1, labels_2))
		  {
		      is_compatible = false;
		      break;
                  }
		}
	    }

	  if(is_compatible)
	    {
	      new_group = false;
	      idx_of_group_to_which_added = idx;
	      dpe_group[idx].insert(dpe);
	      break;
	    }
	}
    }

  if(new_group)
    {

      idx_of_group_to_which_added = dpe_group.size();
      set<vcDatapathElement*> nset;
      nset.insert(dpe);
      dpe_group.push_back(nset);
    }

   if(vcSystem::_verbose_flag)
   {
	std::cerr << "Info: " << dpe->Get_Id();
	if(!vcSystem::_min_area_flag)
	{
		std::cerr << " (";
		for(int i = 0, i_f = labels_1.size(); i < i_f; i++)
		{
			std::cerr << " " << labels_1[i]->Get_Id();
		}
		std::cerr << ") ";
	}
	std::cerr << " included in " 
		  << dpe->Kind() << " group "
		  << idx_of_group_to_which_added << endl;
   }
}


void vcDataPath::Print_Compatible_Operator_Groups(ostream& ofile)
{
  ofile << "Compatible share-able operator groups " << endl;
  this->Print_Compatible_Operator_Groups(ofile,_compatible_split_operator_groups);
  this->Print_Compatible_Operator_Groups(ofile,_compatible_load_groups);
  this->Print_Compatible_Operator_Groups(ofile,_compatible_store_groups);
  this->Print_Compatible_Operator_Groups(ofile,_compatible_call_groups);
  this->Print_Compatible_Operator_Groups(ofile,_compatible_outport_groups);
  this->Print_Compatible_Operator_Groups(ofile,_compatible_inport_groups);
}

void vcDataPath::Print_Compatible_Operator_Groups(ostream& ofile, vector<set<vcDatapathElement*> >& dpe_groups)
{

  for(int idx = 0; idx < dpe_groups.size(); idx++)
    {
      ofile << "Operator " << (*(dpe_groups[idx].begin()))->Get_Operator_Type() << endl;
      ofile << "{ " << endl;
      for(set<vcDatapathElement*>::iterator iter = dpe_groups[idx].begin();
	  iter != dpe_groups[idx].end();
	  iter++)
	{
	  ofile << (*iter)->Get_Id() << "  ";
	  ofile << "   ("
		<< (*iter)->_reqs.front()->Get_Compatibility_Label()->Get_Id()
		<< ","
		<< (*iter)->_acks.back()->Get_Compatibility_Label()->Get_Id()
		<< ")" << endl;
	}
      
      ofile << "} " << endl;
    }
}



//
// for each memory space accessed by this module
// add interface ports.
//
// the width of the interface ports is determined by number
// of load/store groups associated with the memory space.
string vcDataPath::Print_VHDL_Memory_Interface_Ports(string semi_colon, ostream& ofile)
{
  map<vcMemorySpace*, vector<int> > ms_to_load_group_map;
  vcMemorySpace* ms = NULL;

  
  // Each load group to a memory space contributes one request 
  // and one complete bundle.
  for(int idx = 0; idx < _compatible_load_groups.size(); idx++)
    {
      ms = ((vcLoad*) (*(_compatible_load_groups[idx].begin())))->Get_Memory_Space();
      if(ms->Get_Scope() == NULL)
	ms_to_load_group_map[ms].push_back(idx);
    }

  // for each memory space, add a port with the appropriate
  // width..
  for(map<vcMemorySpace*,vector<int> >::iterator ms_iter = ms_to_load_group_map.begin();
      ms_iter != ms_to_load_group_map.end();
      ms_iter++)
    {
      ofile << semi_colon << endl;

      ms = (*ms_iter).first;

      int num_reqs = (*ms_iter).second.size();
      int tag_width = ms->Get_Tag_Length();
      int time_stamp_width = ms->Calculate_Time_Stamp_Width();

      // load ports to this memory space from this module
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_req") 
	    << " : out  std_logic_vector(" << num_reqs-1  << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_ack") 
	    << " : in   std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_addr") 
	    << " : out  std_logic_vector(" << (num_reqs*ms->Get_Address_Width())-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_tag") 
	    << " :  out  std_logic_vector(" << (num_reqs*(time_stamp_width+tag_width))-1  << " downto 0);" << endl;

      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_req") 
	    << " : out  std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_ack") 
	    << " : in   std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_data") 
	    << " : in   std_logic_vector(" << (num_reqs*ms->Get_Word_Size())-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_tag") 
	    << " :  in  std_logic_vector("  << (num_reqs*tag_width)-1  << " downto 0)";
      semi_colon = ";";
    }

  map<vcMemorySpace*, vector<int> > ms_to_store_group_map;

  // Each load group to a memory space contributes one request 
  // and one complete bundle.
  for(int idx = 0; idx < _compatible_store_groups.size(); idx++)
    {
      ms = ((vcStore*) (*(_compatible_store_groups[idx].begin())))->Get_Memory_Space();
      if(ms->Get_Scope() == NULL)
	ms_to_store_group_map[ms].push_back(idx);
    }

  // for each memory space, add a port with the appropriate
  // width..
  for(map<vcMemorySpace*,vector<int> >::iterator ms_iter = ms_to_store_group_map.begin();
      ms_iter != ms_to_store_group_map.end();
      ms_iter++)
    {
      ofile << semi_colon << endl;

      int num_stores = (*ms_iter).first->Get_Num_Stores();

      ms = (*ms_iter).first;

      int num_reqs = (*ms_iter).second.size();
      int tag_width = ms->Get_Tag_Length();
      int time_stamp_width = ms->Calculate_Time_Stamp_Width();

      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_req") 
	    << " : out  std_logic_vector(" << num_reqs-1  << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_ack") 
	    << " : in   std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_addr") 
	    << " : out  std_logic_vector(" << (num_reqs*ms->Get_Address_Width())-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_data") 
	    << " : out  std_logic_vector(" << (num_reqs*ms->Get_Word_Size())-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_tag") 
	    << " :  out  std_logic_vector(" << (num_reqs*(tag_width+time_stamp_width))-1  << " downto 0);" << endl;

      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sc_req") 
	    << " : out  std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sc_ack") 
	    << " : in   std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sc_tag") 
	    << " :  in  std_logic_vector("  << (num_reqs*tag_width)-1  << " downto 0)";
      semi_colon = ";";
    }
  return(semi_colon);
}

string vcDataPath::Print_VHDL_IO_Interface_Ports(string semi_colon, ostream& ofile)
{
  map<vcPipe*, vector<int> > pipe_to_inport_group_map;
  string pipe_id;
  int word_size;

  for(int idx = 0; idx < _compatible_inport_groups.size(); idx++)
    {
      vcPipe* p = ((vcInport*) 
		   (*(_compatible_inport_groups[idx].begin())))->Get_Pipe();
      pipe_to_inport_group_map[p].push_back(idx);
    }

  for(map<vcPipe*,vector<int> >::iterator ms_iter = pipe_to_inport_group_map.begin();
      ms_iter != pipe_to_inport_group_map.end();
      ms_iter++)
    {
      vcPipe* p = (*ms_iter).first;
      if(p->Get_Parent() == NULL)
	{
	  word_size = p->Get_Width();
	  string pipe_id = p->Get_Id();
	  
	  int num_reqs = (*ms_iter).second.size();
	  ofile << semi_colon << endl;

	  
	  ofile << this->Get_VHDL_IOport_Interface_Port_Name(pipe_id,"read_req")
		<< " : out  std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
	  ofile << this->Get_VHDL_IOport_Interface_Port_Name(pipe_id,"read_ack")
		<< " : in   std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
	  ofile << this->Get_VHDL_IOport_Interface_Port_Name(pipe_id,"read_data")
		<< " : in   std_logic_vector(" << (num_reqs * word_size)-1 << " downto 0)";
	  
	  semi_colon = ";";
	}
    }


  map<vcPipe*, vector<int> > pipe_to_outport_group_map;
  for(int idx = 0; idx < _compatible_outport_groups.size(); idx++)
    {
      vcPipe* p = ((vcOutport*) (*(_compatible_outport_groups[idx].begin())))->Get_Pipe();
      pipe_to_outport_group_map[p].push_back(idx);
    }

  for(map<vcPipe*,vector<int> >::iterator ms_iter = pipe_to_outport_group_map.begin();
      ms_iter != pipe_to_outport_group_map.end();
      ms_iter++)
    {
      vcPipe* p = (*ms_iter).first;
      if(p->Get_Parent() == NULL)
	{
	  string pipe_id = p->Get_Id();
	  word_size = p->Get_Width();

	  ofile << semi_colon << endl;

	  int num_reqs = (*ms_iter).second.size();
	  ofile << this->Get_VHDL_IOport_Interface_Port_Name(pipe_id,"write_req")
		<< " : out  std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
	  ofile << this->Get_VHDL_IOport_Interface_Port_Name(pipe_id,"write_ack")
		<< " : in   std_logic_vector(" << num_reqs-1 << " downto 0);" << endl;
	  ofile << this->Get_VHDL_IOport_Interface_Port_Name(pipe_id,"write_data")
		<< " : out  std_logic_vector(" << (num_reqs * word_size)-1 << " downto 0)";
	  semi_colon = ";";
	}
    }
  return(semi_colon);
}

string  vcDataPath::Print_VHDL_Call_Interface_Ports(string semi_colon, ostream& ofile)
{
  map<vcModule*, vector<int> > called_module_to_call_group_map;
  vcModule* called_module;

  // Each call group to a module contributes one request 
  // and one complete bundle.
  for(int idx = 0; idx < _compatible_call_groups.size(); idx++)
    {
      called_module = ((vcCall*) (*(_compatible_call_groups[idx].begin())))->Get_Called_Module();
      called_module_to_call_group_map[called_module].push_back(idx);
    }

  // for each call group
  // width..
  for(map<vcModule*,vector<int> >::iterator called_module_iter = called_module_to_call_group_map.begin();
      called_module_iter != called_module_to_call_group_map.end();
      called_module_iter++)
    {

      ofile << semi_colon << endl;

      called_module = (*called_module_iter).first;

      int num_reqs = (*called_module_iter).second.size();
      int tag_width = called_module->Get_Caller_Tag_Length();
      
      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_reqs")
	    << " : out  std_logic_vector(" << num_reqs-1 <<  " downto 0);" << endl;
      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_acks") 
	    << " : in   std_logic_vector(" << num_reqs-1 <<  " downto 0);" << endl;

      if(called_module->Get_In_Arg_Width() > 0)
	ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_data") 
	      << " : out  std_logic_vector(" << (num_reqs * called_module->Get_In_Arg_Width())-1 << " downto 0);" << endl;

      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_tag") 
	    << "  :  out  std_logic_vector(" << (num_reqs * tag_width)-1 << " downto 0);" << endl;
      
      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_reqs") 
	    << " : out  std_logic_vector(" << num_reqs-1 <<  " downto 0);" << endl;
      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_acks") 
	    << " : in   std_logic_vector(" << num_reqs-1 <<  " downto 0);" << endl;
      if(called_module->Get_Out_Arg_Width() > 0)
	ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_data") 
	      << " : in   std_logic_vector(" << (num_reqs * called_module->Get_Out_Arg_Width())-1 << " downto 0);" << endl;
      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_tag") 
	    << " :  in   std_logic_vector(" << (num_reqs * tag_width)-1 << " downto 0)";
      semi_colon = ";";
    }
  return(semi_colon);

}


void vcDataPath::Print_VHDL(ostream& ofile)
{
  ofile << "data_path: Block -- { " << endl;

  // print wires.
  for(map<string, vcWire*>::iterator iter = _wire_map.begin();
      iter != _wire_map.end();
      iter++)
    {
      (*iter).second->Print_VHDL_Std_Logic_Declaration(ofile);
    }

  this->Get_Parent()->Print_VHDL_Pipe_Signals(ofile);

    
  ofile << "-- }" << endl << "begin -- { " << endl;

  // tie constant wires to their values.
  for(map<string, vcWire*>::iterator iter = _wire_map.begin();
      iter != _wire_map.end();
      iter++)
    {
      if((*iter).second->Is("vcConstantWire"))
	ofile << ((*iter).second)->Get_VHDL_Signal_Id() << " <= " << ((vcConstantWire*)((*iter).second))->Get_Value()->To_VHDL_String() << ";" << endl;
    }

  // now instantiate each group. 
  this->Print_VHDL_Phi_Instances(ofile); // done
  this->Print_VHDL_Select_Instances(ofile); // done
  this->Print_VHDL_Slice_Instances(ofile);  // done.
  this->Print_VHDL_Permutation_Instances(ofile);  // done.
  this->Print_VHDL_Register_Instances(ofile); // done (no changes)
  this->Print_VHDL_Interlock_Buffer_Instances(ofile); // done.
  this->Print_VHDL_Equivalence_Instances(ofile); // done (no changes)
  this->Print_VHDL_Branch_Instances(ofile);  // done (no changes)
  this->Print_VHDL_Split_Operator_Instances(ofile); // done
  this->Print_VHDL_Load_Instances(ofile); // done
  this->Print_VHDL_Store_Instances(ofile); // done
  this->Get_Parent()->Print_VHDL_Pipe_Instances(ofile); // done (no changes)
  this->Print_VHDL_Inport_Instances(ofile); // done
  this->Print_VHDL_Outport_Instances(ofile); // done
  this->Print_VHDL_Call_Instances(ofile); // done


  ofile << "-- }" << endl << "end Block; -- data_path" << endl;
}


void vcDataPath::Print_VHDL_Phi_Instances(ostream& ofile)
{ 
  for(map<string, vcPhi*>::iterator iter = _phi_map.begin();
      iter != _phi_map.end();
      iter++)
    {
      vcPhi* p = (*iter).second;
      if(vcSystem::_enable_logging)
      {
	p->vcDatapathElement::Print_VHDL_Logger(ofile);
      }

      p->Print_VHDL(ofile);
    }
}





void vcDataPath::Print_VHDL_Select_Instances(ostream& ofile)
{ 
  int idx = 0;
  for(map<string, vcSelect*>::iterator iter = _select_map.begin();
      iter != _select_map.end();
      iter++)
    {
      vcSelect* s = (*iter).second;
      if(vcSystem::_enable_logging)
		s->vcSplitOperator::Print_VHDL_Logger(ofile);

      s->Print_VHDL(ofile);
      idx++;
   }
}


void vcDataPath::Print_VHDL_Slice_Instances(ostream& ofile)
{ 
  int idx = 0;

  for(map<string, vcSlice*>::iterator iter = _slice_map.begin();
      iter != _slice_map.end();
      iter++)
    {
      vcSlice* s = (*iter).second;
      if(vcSystem::_enable_logging)
		s->vcSplitOperator::Print_VHDL_Logger(ofile);

      s->Print_VHDL(ofile);
      idx++;
    }
}

void vcDataPath::Print_VHDL_Permutation_Instances(ostream& ofile)
{ 
  int idx = 0;

  for(map<string, vcPermutation*>::iterator iter = _permutation_map.begin();
      iter != _permutation_map.end();
      iter++)
    {
      vcPermutation* s = (*iter).second;
      if(vcSystem::_enable_logging)
		s->vcSplitOperator::Print_VHDL_Logger(ofile);

      s->Print_VHDL(ofile);
      idx++;
    }
}

void vcDataPath::Print_VHDL_Register_Instances(ostream& ofile)
{ 

  int idx = 0;
  for(map<string, vcRegister*>::iterator iter = _register_map.begin();
      iter != _register_map.end();
      iter++)
    {
      vcRegister* s = (*iter).second;
      if(vcSystem::_enable_logging)
	s->vcOperator::Print_VHDL_Logger(ofile);
      s->Print_VHDL(ofile);
      idx++;
  }
}

void vcDataPath::Print_VHDL_Interlock_Buffer_Instances(ostream& ofile)
{
  for(map<string, vcInterlockBuffer*>::iterator iter = _interlock_buffer_map.begin();
      iter != _interlock_buffer_map.end();
      iter++)
    {
      vcInterlockBuffer* p = (*iter).second;
      if(vcSystem::_enable_logging)
      {
	p->vcDatapathElement::Print_VHDL_Logger(ofile);
      }

      p->Print_VHDL(ofile);
    }
}


void vcDataPath::Print_VHDL_Equivalence_Instances(ostream& ofile)
{ 

  for(map<string, vcEquivalence*>::iterator iter = _equivalence_map.begin();
      iter != _equivalence_map.end();
      iter++)
    {
      vcEquivalence* s = (*iter).second;
      if(vcSystem::_enable_logging)
		s->vcOperator::Print_VHDL_Logger(ofile);
      ofile << s->Get_VHDL_Id() << ": Block -- { " << endl;
      ofile << "signal in_aggregated_sig: std_logic_vector("
	    << s->_in_width-1 << " downto 0);" << endl;
      ofile << "signal out_aggregated_sig: std_logic_vector("
	    << s->_out_width-1 << " downto 0);" << endl;
      if(s->_out_width > s->_in_width)
      {
	ofile << "constant in_pad : std_logic_vector(" << 
		(s->_out_width-s->_in_width)-1 << " downto 0) := (others => '0');" << endl;
      }
      ofile <<  "--}" << endl;
      ofile << "begin -- {" << endl;
      ofile << s->Get_Ack(0)->Get_DP_To_CP_Symbol()  
	    << " <= "
	    << s->Get_Req(0)->Get_CP_To_DP_Symbol() 
	    << ";" << endl;
      ofile << " in_aggregated_sig <= ";
      for(int idx = 0; idx < s->_inwires.size(); idx++)
	{
	  if(idx > 0)
	    ofile << " & ";
	  ofile << s->_inwires[idx]->Get_VHDL_Signal_Id();
	}
      ofile << ";" << endl;

      if(s->_out_width > s->_in_width)
      {
	ofile << "out_aggregated_sig <= in_pad & in_aggregated_sig; " << endl;
      }
      else if(s->_out_width < s->_in_width)
      {
	ofile << "out_aggregated_sig <= in_aggregated_sig(" << s->_out_width-1 << " downto 0);" << endl;
      }
      else
	ofile << "out_aggregated_sig <= in_aggregated_sig;" << endl;


      int top_index = s->_out_width-1;
      for(int idx = 0; idx < s->_outwires.size(); idx++)
	{
	  ofile << s->_outwires[idx]->Get_VHDL_Signal_Id() 
		<< " <= out_aggregated_sig("
		<< top_index
		<< " downto "
		<< (top_index - s->_outwires[idx]->Get_Size())+1
		<< ");" << endl;
	  top_index -= s->_outwires[idx]->Get_Size();
	}
      ofile << "--}" << endl;
      ofile << "end Block;" << endl;
    }
}

void vcDataPath::Print_VHDL_Branch_Instances(ostream& ofile)
{ 
  for(map<string, vcBranch*>::iterator iter = _branch_map.begin();
      iter != _branch_map.end();
      iter++)
    {
      vcBranch* s = (*iter).second;
      if(vcSystem::_enable_logging)
		s->vcDatapathElement::Print_VHDL_Logger(ofile);

      int in_width = 0;
      for(int idx = 0; idx < s->_inwires.size(); idx++)
	in_width += s->_inwires[idx]->Get_Size();

      ofile << s->Get_VHDL_Id() << ": Block -- { -- branch-block" << endl;
      ofile << "signal condition_sig : std_logic_vector(" << in_width-1 << " downto 0);" << endl;
      ofile << "begin " << endl;
      ofile << "condition_sig <= ";
      for(int idx = 0; idx < s->_inwires.size(); idx++)
	{
	  if(idx > 0)
	    ofile << " & ";
	  ofile << s->_inwires[idx]->Get_VHDL_Signal_Id();
	}
      ofile << ";" << endl;
      ofile << "branch_instance: BranchBase -- {" << endl;
      ofile << " generic map( condition_width => " << in_width << ")" << endl;
      ofile << " port map( -- { " << endl << " condition => condition_sig";
      ofile << "," << endl;
      ofile << "req => " << s->Get_Req(0)->Get_CP_To_DP_Symbol() << "," <<  endl
	    << "ack0 => " << ((s->Get_Ack(0) != NULL) ? s->Get_Ack(0)->Get_DP_To_CP_Symbol() : 
			      "open" )
	    << "," << endl
	    << "ack1 => " << ((s->Get_Ack(1) != NULL) ? s->Get_Ack(1)->Get_DP_To_CP_Symbol() : 
			      "open" )
	    << "," << endl
	    << "clk => clk," << endl
	    << "reset => reset); -- }}" << endl;
      ofile << "--}\n end Block; -- branch-block" << endl;
    }
}

//
// A hell of an ugly function.. Can it be improved?
// Basically, look at each operator group,
// collect the information for the inputs and outputs
// and loads of other stuff.  There are three distinct
// case:
//    1:  Pipelined FP operator (shared)
//    2:  Unshared operator (not pipelined FP)
//    3.  Shared operator (not pipelined FP)
//
// Break it up into three functions depending on the case?
// We just need to add input-buffering to it for now..
// Not worth touching at this point.
void vcDataPath::Print_VHDL_Split_Operator_Instances(ostream& ofile)
{
  string group_name;
  string no_arb_string; 

  for(int idx = 0; idx < this->_compatible_split_operator_groups.size(); idx++)
    { // for each operator group.
      
      bool is_unary_operator = false;
      // number of requesters.
      int num_reqs = _compatible_split_operator_groups[idx].size();

      // to collect inwires, outwires and reqs/acks.
      vector<vcWire*> inwires;
      vector<int> inwire_buffering;

      vector<vcTransition*> reqL;
      vector<vcTransition*> ackL;
      vector<vcTransition*> reqR;
      vector<vcTransition*> ackR;

      vector<vcWire*> outwires;
      vector<int> outwire_buffering;

      vector<vcWire*> guard_wires;
      vector<bool> guard_complements;



      // to get the operation id, we need the vc operator as well as the input and output 
      // types. (e.g. + (float float) (float) or + (int int) (int) ?
      vcSplitOperator* lead_op = ((vcSplitOperator*)(*(_compatible_split_operator_groups[idx].begin())));
      vcType* input_type =   lead_op->Get_Input_Type();
      vcType* output_type =   lead_op->Get_Output_Type();
      string vc_op_id = lead_op->Get_Op_Id();
      string vhdl_op_id = Get_VHDL_Op_Id(vc_op_id,
					 input_type,
					 output_type);

      string s__id = StripBracketingQuotes(vhdl_op_id);
      group_name = s__id + "_group_" + IntToStr(idx) ;
      
      bool is_pipelined_op = lead_op->Is_Pipelined_Operator();

       // for pipelined ops.
      if(is_pipelined_op)
      {
	no_arb_string = "false";
      }
      else
      {
	no_arb_string = (vcSystem::_min_area_flag ? "false" : "true");
      }

      // the number of inputs and outputs on each operator in the current group.
      int num_ips = ((vcSplitOperator*)(*(_compatible_split_operator_groups[idx].begin())))->Get_Number_Of_Input_Wires();
      int num_ops = ((vcSplitOperator*)(*(_compatible_split_operator_groups[idx].begin())))->Get_Number_Of_Output_Wires();
					 
      // to keep track of the ids of the operators in this group.
      vector<string> elements;

      // to keep track of the operators in this group.
      vector<vcDatapathElement*> dpe_elements;


      // ok. collect all the information..
      for(set<vcDatapathElement*>::iterator iter = _compatible_split_operator_groups[idx].begin();
	  iter != _compatible_split_operator_groups[idx].end();
	  iter++)
	{

	  vcSplitOperator* so = (vcSplitOperator*) (*iter);
	  if(vcSystem::_enable_logging)
		so->vcSplitOperator::Print_VHDL_Logger(ofile);
	  is_unary_operator = Is_Unary_Op(so->Get_Op_Id());

	  dpe_elements.push_back(so);
	  elements.push_back(so->Get_VHDL_Id());

	  so->Append_Inwires(inwires);
	  so->Append_Inwire_Buffering(inwire_buffering);

	  so->Append_Outwires(outwires);
	  so->Append_Outwire_Buffering(outwire_buffering);
          
	  reqL.push_back(so->Get_Req(0));
	  ackL.push_back(so->Get_Ack(0));
	  reqR.push_back(so->Get_Req(1));
	  ackR.push_back(so->Get_Ack(1));

	  so->Append_Guard(guard_wires,guard_complements);
	}

      //string buffering_string;
      //this->Generate_Buffering_Constant_Declaration(dpe_elements, buffering_string);
      
      string input_buffering_string;
      int max_inbuf = this->Generate_Buffering_String("inBUFs", inwire_buffering, input_buffering_string);

      string output_buffering_string;
      int max_outbuf = this->Generate_Buffering_String("outBUFs", outwire_buffering, output_buffering_string);

      string guard_flags;
      string guard_buffering;
      Generate_Guard_Constants(guard_buffering, guard_flags, dpe_elements, guard_wires);

      // is the second input a constant?
      bool use_constant = false;
      string const_operand;
      int const_width = 1;
      if((!is_pipelined_op) && (num_ips == 2 && inwires[1]->Is("vcConstantWire")))
	{
	  num_ips = 1; // has only one input, we will be using one constant operand.
	  use_constant = true;
	  const_operand = ((vcConstantWire*)inwires[1])->Get_Value()->To_VHDL_String();
	  const_width = ((vcConstantWire*)inwires[1])->Get_Size();
	}
      else
	{
	  const_operand += "\"0\"";
	}


      // total in-width 
      int in_width = 0;
      vector<vcWire*> concat_in_wires;
      for(int u = 0; u < inwires.size(); u++)
	{
	  if(!use_constant || (u%2 == 0))
	    {
	      // do not count if inwire is
	      // an even operand and if it is
	      // a constant.
	      in_width += inwires[u]->Get_Size();
	      concat_in_wires.push_back(inwires[u]);
	    }
	}


      // total out-width..
      int out_width = 0;
      for(int u = 0; u < outwires.size(); u++)
	{
	  out_width += outwires[u]->Get_Size();
	}

      // VHDL code for this shared group
      ofile << "-- shared split operator group (" << idx << ") : " ;
      for(int u = 0; u < elements.size(); u++)
	ofile << elements[u] << " ";
      ofile << endl;

	bool use_regulator = false;
      if((num_reqs > 1) || is_pipelined_op)
	{
		use_regulator = true;
	}

      // make a block
      ofile <<  group_name << ": Block -- {" << endl;
      // in and out data.
      ofile << "signal data_in: std_logic_vector(" << in_width-1 << " downto 0);" << endl;
      ofile << "signal data_out: std_logic_vector(" << out_width-1 << " downto 0);" << endl;
      // in and out acks.
      ofile << "signal reqR, ackR, reqL, ackL : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqR_unguarded, ackR_unguarded, reqL_unguarded, ackL_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      if(use_regulator)
	      ofile << "signal reqL_unregulated, ackL_unregulated : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal guard_vector : std_logic_vector( " << num_reqs-1 << " downto 0);" << endl;

      ofile << input_buffering_string << endl;
      ofile << output_buffering_string << endl;
      ofile << guard_flags << endl;
      ofile << guard_buffering << endl;


      ofile << "-- }\n begin -- {" << endl;

      // concatenate data_in
      Print_VHDL_Concatenation(string("data_in"), concat_in_wires,ofile);

      // disconcatenate data_out
      Print_VHDL_Disconcatenation(string("data_out"), out_width, outwires,ofile);

      	// prepare guard vector.
      Print_VHDL_Guard_Concatenation(num_reqs, "guard_vector", guard_wires, guard_complements, ofile);

      // input types 
      vcType* input_type_1 = inwires[0]->Get_Type();
      vcType* input_type_2 = input_type_1;

      if(num_ips == 2)
	input_type_2 = inwires[1]->Get_Type();

	// used in the unshared case.
      bool flow_through = lead_op->Get_Flow_Through();
	
      // the guards and the regulators..
      if((num_reqs > 1) || is_pipelined_op)
	{
	  Print_VHDL_Concatenate_Req("reqL_unguarded",reqL,ofile);
	  Print_VHDL_Disconcatenate_Ack("ackL_unguarded",ackL,ofile);
	  Print_VHDL_Concatenate_Req("reqR_unguarded",reqR,ofile);
	  Print_VHDL_Disconcatenate_Ack("ackR_unguarded",ackR,ofile);

	  this->Print_VHDL_Regulator_Instance(group_name + "_accessRegulator", num_reqs,"reqL_unregulated", "ackL_unregulated", "reqL", "ackL", "reqR", "ackR", dpe_elements, ofile);
	/*
	  this->Print_VHDL_Guard_Instance("gI0", num_reqs,"guard_vector", "reqL_unguarded", "ackL_unguarded", "reqL_unregulated", "ackL_unregulated", false, ofile);
      	  this->Print_VHDL_Guard_Instance("gI1", num_reqs,"guard_vector", "reqR_unguarded", "ackR_unguarded", "reqR", "ackR", true, ofile);
	*/
	Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
						"reqL_unguarded", "ackL_unguarded",
						"reqL_unregulated", "ackL_unregulated",
						"reqR_unguarded", "ackR_unguarded",
						"reqR", "ackR", ofile);

	}
      else
	{
	  // an unshared operator.
	  Print_VHDL_Concatenate_Req("reqL_unguarded",reqL,ofile);
	  Print_VHDL_Disconcatenate_Ack("ackL_unguarded",ackL,ofile);
	  Print_VHDL_Concatenate_Req("reqR_unguarded",reqR,ofile);
	  Print_VHDL_Disconcatenate_Ack("ackR_unguarded",ackR,ofile);

	  if(!flow_through)
	  {
		/*
	  	this->Print_VHDL_Guard_Instance("gI0", num_reqs,"guard_vector", "reqL_unguarded", "ackL_unguarded", "reqL", "ackL", false, ofile);
      	  	this->Print_VHDL_Guard_Instance("gI1", num_reqs,"guard_vector", "reqR_unguarded", "ackR_unguarded", "reqR", "ackR", true, ofile);
		*/
		Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
						"reqL_unguarded", "ackL_unguarded",
						"reqL", "ackL",
						"reqR_unguarded", "ackR_unguarded",
						"reqR", "ackR", ofile);
	  }
	  else
	  {
		ofile << "reqL <= reqL_unguarded;" << endl;
		ofile << "reqR <= reqR_unguarded;" << endl;
		ofile << "ackL_unguarded <= ackL;" << endl;
		ofile << "ackR_unguarded <= ackR;" << endl;
	  }

	  /*
	  if(guard_wires[0] != NULL)
	    {
	      ofile << "reqL(0) <= " << reqL[0]->Get_CP_To_DP_Symbol() 
		    << " when " << guard_wires[0]->Get_VHDL_Id() 
		    << "(0) = " << (guard_complements[0] ? "'0'"  : "'1'") << " else false;" << endl; 
	      ofile << ackL[0]->Get_DP_To_CP_Symbol() << " <= ackL(0) " 
		    << " when " << guard_wires[0]->Get_VHDL_Id()
		    << "(0) = " << (guard_complements[0] ? "'0'"  : "'1'") << " else " 
		    << reqL[0]->Get_CP_To_DP_Symbol()  << ";" 
		    << endl; 
	      
	      ofile << "reqR(0) <= " << reqR[0]->Get_CP_To_DP_Symbol() 
		    << " when " << guard_wires[0]->Get_VHDL_Id() 
		    << "(0) = " << (guard_complements[0] ? "'0'"  : "'1'") << " else false;" << endl; 
	      ofile << ackR[0]->Get_DP_To_CP_Symbol() << " <= ackR(0) " 
		    << " when " << guard_wires[0]->Get_VHDL_Id()
		    << "(0) = " << (guard_complements[0] ? "'0'"  : "'1'") << " else " 
		    << reqR[0]->Get_CP_To_DP_Symbol()  << ";" 
		    << endl; 
	    }
	  else
	    {
	      ofile << "reqL(0) <= " << reqL[0]->Get_CP_To_DP_Symbol() << ";" << endl;
	      ofile << "reqR(0) <= " << reqR[0]->Get_CP_To_DP_Symbol() << ";" << endl;
	      ofile << ackL[0]->Get_DP_To_CP_Symbol() << " <= ackL(0); "  << endl;
	      ofile << ackR[0]->Get_DP_To_CP_Symbol() << " <= ackR(0); "  << endl;
	    }
		*/
	}
      

      // now the operator instance.
      // it can be an instance either of 
      //   1. a shared operator. 
      //   2. an unshared operator.
      //   3. a pipelined FP (ieee754) operator.
      // 
      // This stuff can be simplified considerably by just 
      // moving all the complexity to the VHDL library.
      // This is probably the right solution, because it decouples
      // vc2vhdl from the VHDL library.
      if(is_pipelined_op)
	{
	   // for the moment.
	  assert(input_type == output_type);
	  assert(input_type->Is_Float_Type() && output_type->Is_Float_Type());

	  int exp_width =  ((vcFloatType*) input_type)->Get_Characteristic_Width();
          int frac_width =  ((vcFloatType*) input_type)->Get_Mantissa_Width();

	  ofile << "PipedFpOp: PipelinedFPOperator -- {" << endl;
	  ofile << " generic map( -- { " << endl 
		<< " name => " << '"' << group_name << '"' << "," << endl 
		<< " operator_id => " << vhdl_op_id << "," << endl
		<< " exponent_width => " << exp_width << "," << endl
		<< " fraction_width => " << frac_width << ", " << endl
		<< " no_arbitration => " << no_arb_string << ","  << endl
		<< " num_reqs => " << num_reqs << "," << endl
		<< " use_input_buffering => true," << endl
		<< " detailed_buffering_per_input => inBUFs," << endl
		<< " detailed_buffering_per_output => outBUFs -- } \n )" << endl;
	  ofile << "port map ( reqL => reqL , ackL => ackL, reqR => reqR, ackR => ackR, dataL => data_in, dataR => data_out, clk => clk, reset => reset); -- }" << endl;
	  ofile << "-- } \n end Block; -- split operator group " << idx << endl; // thats it..
	}
      else
	{
	  if(num_reqs > 1)
	    {
	      // ok, a shared operator

	      ofile << "SplitOperator: SplitOperatorShared -- {" << endl;
	      ofile << "generic map ( -- { " ;
	      ofile << " name => " << '"' << group_name << '"' << "," << endl;
	      // a ton of generics..
	      ofile << " operator_id => " << vhdl_op_id << "," << endl  // operator-id
		    << " input1_is_int => " << (input_type_1->Is_Int_Type() ? "true" : "false") << ", " << endl  // first op is int?
		    << " input1_characteristic_width => " 
		    << (input_type_1->Is("vcFloatType") ? ((vcFloatType*)input_type_1)->Get_Characteristic_Width() : 0) << ", " << endl // char 1
		    << " input1_mantissa_width    => " 
		    << (input_type_1->Is("vcFloatType") ? ((vcFloatType*)input_type_1)->Get_Mantissa_Width() : 0) << ", " << endl // mantissa 1
		    << " iwidth_1  => " << input_type_1->Size() << "," << endl // width 1
		    << " input2_is_int => " << (input_type_2->Is_Int_Type() ? "true" : "false") << ", " << endl  // second op is int?
		    << " input2_characteristic_width => " 
		    << (input_type_2->Is("vcFloatType") ? ((vcFloatType*)input_type_2)->Get_Characteristic_Width() : 0) << ", " << endl // char 2
		    << " input2_mantissa_width => "
		    << (input_type_2->Is("vcFloatType") ? ((vcFloatType*)input_type_2)->Get_Mantissa_Width() : 0) << ", " << endl // mantissa 2
		    << " iwidth_2      => " << (num_ips == 2 ? input_type_2->Size() : 0) << ", " << endl // width 2
		    << " num_inputs    => " << num_ips << ","  << endl // number of inputs
		    << " output_is_int => " << (output_type->Is_Int_Type() ? "true" : "false") << "," << endl // output is int?
		    << " output_characteristic_width  => " 
		    << (output_type->Is("vcFloatType") ? ((vcFloatType*)output_type)->Get_Characteristic_Width() : 0) << ", " << endl // char op
		    << " output_mantissa_width => " 
		    << (output_type->Is("vcFloatType") ? ((vcFloatType*)output_type)->Get_Mantissa_Width() : 0) << ", " << endl // mantissa op
		    << " owidth => " << output_type->Size() << "," << endl  // output width
		    << " constant_operand => " << const_operand << "," << endl // constant operand?
		    << " constant_width => " << const_width << "," << endl // constant width
		    << " use_constant  => " << (use_constant ? "true" : "false") << "," << endl // use constant?
		    << " no_arbitration => " << no_arb_string << "," << endl
		    << " min_clock_period => " << (vcSystem::_min_clock_period_flag ? "true" : "false") << "," << endl
		    << " num_reqs => " << num_reqs << "," << endl
		    << " use_input_buffering => true," << endl
		    << " detailed_buffering_per_input => inBUFs," << endl
		    << " detailed_buffering_per_output => outBUFs --} \n )" << endl; // number of requesters..
	      ofile << "port map ( reqL => reqL , ackL => ackL, reqR => reqR, ackR => ackR, dataL => data_in, dataR => data_out, clk => clk, reset => reset); -- }" << endl;
	    }
	  else
	    {
	      string iname = '"' + group_name + '"';
	      int bufv = ( (max_inbuf > max_outbuf) ? max_inbuf : max_outbuf);
	      ofile << "UnsharedOperator: UnsharedOperatorWithBuffering -- {" << endl;
	      ofile << "generic map ( -- { " ;
	  
	      // a ton of generics..
	      ofile << " operator_id => " << vhdl_op_id << "," << endl  // operator-id
		    << " name => " << iname << "," << endl
		    << " input1_is_int => " << (input_type_1->Is_Int_Type() ? "true" : "false") << ", " << endl  // first op is int?
		    << " input1_characteristic_width => " 
		    << (input_type_1->Is("vcFloatType") ? ((vcFloatType*)input_type_1)->Get_Characteristic_Width() : 0) << ", " << endl // char 1
		    << " input1_mantissa_width    => " 
		    << (input_type_1->Is("vcFloatType") ? ((vcFloatType*)input_type_1)->Get_Mantissa_Width() : 0) << ", " << endl // mantissa 1
		    << " iwidth_1  => " << input_type_1->Size() << "," << endl // width 1
		    << " input2_is_int => " << (input_type_2->Is_Int_Type() ? "true" : "false") << ", " << endl  // second op is int?
		    << " input2_characteristic_width => " 
		    << (input_type_2->Is("vcFloatType") ? ((vcFloatType*)input_type_2)->Get_Characteristic_Width() : 0) << ", " << endl // char 2
		    << " input2_mantissa_width => "
		    << (input_type_2->Is("vcFloatType") ? ((vcFloatType*)input_type_2)->Get_Mantissa_Width() : 0) << ", " << endl // mantissa 2
		    << " iwidth_2      => " << (num_ips == 2 ? input_type_2->Size() : 0) << ", " << endl // width 2
		    << " num_inputs    => " << num_ips << ","  << endl // number of inputs
		    << " output_is_int => " << (output_type->Is_Int_Type() ? "true" : "false") << "," << endl // output is int?
		    << " output_characteristic_width  => " 
		    << (output_type->Is("vcFloatType") ? ((vcFloatType*)output_type)->Get_Characteristic_Width() : 0) << ", " << endl // char op
		    << " output_mantissa_width => " 
		    << (output_type->Is("vcFloatType") ? ((vcFloatType*)output_type)->Get_Mantissa_Width() : 0) << ", " << endl // mantissa op
		    << " owidth => " << output_type->Size() << "," << endl  // output width
		    << " constant_operand => " << const_operand << "," << endl // constant operand?
		    << " constant_width => " << const_width << "," << endl // constant width
		    << " buffering  => " << bufv << "," << endl // buffering in operator.
		    << " flow_through => " << (flow_through ? "true" : "false") << "," << endl
		    << " use_constant  => " << (use_constant ? "true" : "false") << endl // use constant?

		    << "--} \n ) " << endl; // number of requesters..
	      ofile << "port map ( -- { " << endl
		    << "reqL => reqL(0)," << endl
		    << "ackL => ackL(0)," << endl
		    << "reqR => reqR(0)," << endl
		    << "ackR => ackR(0)," << endl
		    << "dataL => data_in, " << endl
		    << "dataR => data_out," << endl
		    << "clk => clk," << endl
		    << "reset => reset); -- }}" << endl;
	    }
	  ofile << "-- } \n end Block; -- split operator group " << idx << endl; // thats it..
	}
    }
}



void vcDataPath::Print_VHDL_Regulator_Instance(string inst_id, 
			int num_reqs,  
			string reqs, string acks,
			string regulated_reqs, string regulated_acks, 
			string release_reqs, string release_acks,
			vector<vcDatapathElement*>& dpe_elements,
			ostream& ofile)
{


	// redundancy!
	assert(dpe_elements.size() == num_reqs);

	if(num_reqs == 1)
	{
		ofile << regulated_reqs << " <= " << reqs << ";" << endl;
		ofile << acks << " <= " << regulated_acks << ";" << endl;
		return;
	}

	int idx;
        string inst_id_idx;
	for(idx = 0; idx < num_reqs; idx++)
	{
		inst_id_idx = inst_id + "_" + IntToStr(idx);

		vcDatapathElement* dpe = dpe_elements[(num_reqs-idx)-1];
		
		// needs to get more sophisticated...
		// right now, the same buffering is used for
		// operations in a pipelined loop.
		int depth, num_slots;
		bool cf = dpe->Is_Part_Of_Pipelined_Loop(depth, num_slots);

		ofile << inst_id_idx 
			<< ": access_regulator_base generic map ("
			<< "name => " 
			<< '"' << inst_id_idx  << '"'
			<< ", num_slots => " 
			<< IntToStr(num_slots) <<") -- {" << endl;
		ofile << "port map (req => " << reqs << "(" << idx << "), -- {" << endl; 
		ofile << "ack => " << acks << "(" << idx << ")," << endl;
		ofile << "regulated_req => " << regulated_reqs << "(" << idx << ")," << endl;
		ofile << "regulated_ack => " << regulated_acks << "(" << idx << ")," << endl;
		ofile << "release_req => " << release_reqs << "(" << idx << ")," << endl;
		ofile << "release_ack => " << release_acks << "(" << idx << ")," << endl;
		ofile << "clk => clk, reset => reset); -- }}" << endl;
	}
}

int vcDataPath::Generate_Buffering_String(string const_name,
		vector<int>& buf_sizes, 
		string& buffering_string)
{
	int maxbuf = 0;
	int N = buf_sizes.size();
	buffering_string = "(";
	buffering_string = "constant " + const_name + " : IntegerArray(" + IntToStr(N-1)
		+ " downto 0) := (";
	int J;
	for(J = 0; J <  N; J++)
	{
		if(J > 0)
			buffering_string += ", ";
		int bufv = buf_sizes[J];
		maxbuf = ((bufv > maxbuf) ? bufv : maxbuf);
		buffering_string += IntToStr((N-J)-1) + " => " + IntToStr(bufv);
	}
	buffering_string += ");";
	return(maxbuf);
}

int vcDataPath::Generate_Pipeline_Slot_Demands(vector<vcDatapathElement*>& dpe_elements,
				vector<int>& slot_demands)
{
	int max_buf_size = 0;
	int idx;
	int num_reqs = dpe_elements.size();
	int depth, num_slots;

	if(num_reqs == 1)
	{
	   bool cf = dpe_elements[0]->Is_Part_Of_Pipelined_Loop(depth,num_slots);
	   if(cf)
		slot_demands.push_back(2);
	   else
		slot_demands.push_back(1);
	}

	for(idx = 0; idx < num_reqs;  idx++)
	{
		vcDatapathElement* dpe = dpe_elements[(num_reqs-idx)-1];
		
		// needs to get more sophisticated...
		bool cf = dpe->Is_Part_Of_Pipelined_Loop(depth,num_slots);

		slot_demands.push_back(num_slots);
		max_buf_size = (max_buf_size < num_slots ? num_slots : max_buf_size);
         }
	return(max_buf_size);
}


void vcDataPath::Generate_Buffering_Constant_Declaration(vector<vcDatapathElement*>& dpe_elements, 
							string& buffering_string)
{
	vector<int> buf_sizes;
	int idx;

	int depth, num_slots;
	
	int num_reqs = dpe_elements.size();
	int max_buf_size = Generate_Pipeline_Slot_Demands(dpe_elements, buf_sizes);

	// if single request is present, buffering
	// will be limited to 2.
        if(num_reqs == 1)
	{
	   vcDatapathElement* dpe = dpe_elements[0];
	   bool cf = dpe->Is_Part_Of_Pipelined_Loop(depth,num_slots);
	   if(cf) 
	   	buffering_string = 
			"constant buffering_per_output : IntegerArray(0 downto 0) := (0 => 2);";
	   else
	   	buffering_string = 
			"constant buffering_per_output : IntegerArray(0 downto 0) := (0 => 1);";
	   return;
	}	


	// generate the buffering string..
	int N = buf_sizes.size();
	buffering_string = "(";
	buffering_string = "constant buffering_per_output : IntegerArray(" + IntToStr(N-1)
		+ " downto 0) := (";
	int J;
	for(J = 0; J <  N; J++)
	{
		if(J > 0)
			buffering_string += ", ";
		buffering_string += IntToStr((N-J)-1) + " => " + IntToStr(buf_sizes[J]);
	}
	buffering_string += ");";
}

void vcDataPath::Print_VHDL_Load_Instances(ostream& ofile)
{ 

  string no_arb_string = (vcSystem::_min_area_flag ? "false" : "true");
  // print LoadReqShared instance and LoadCompleteShared instance.
  for(int idx = 0; idx < this->_compatible_load_groups.size(); idx++)
    { // for each operator group.

      // number of requesters.
      int num_reqs = _compatible_load_groups[idx].size();

      // to collect inwires, outwires and reqs/acks.
      vector<vcWire*> inwires;
      vector<int>  inwire_buffering;

      vector<vcWire*> guard_wires;
      vector<bool> guard_complements;
      vector<vcTransition*> reqL;
      vector<vcTransition*> ackL;
      vector<vcTransition*> reqR;
      vector<vcTransition*> ackR;

      vector<vcWire*> outwires;
      vector<int> outwire_buffering;

      // to keep track of the ids of the operators in this group.
      vector<string> elements;
      vector<vcDatapathElement*> dpe_elements;

      vcMemorySpace* ms = NULL;

      // ok. collect all the information..
      for(set<vcDatapathElement*>::iterator iter = _compatible_load_groups[idx].begin();
	  iter != _compatible_load_groups[idx].end();
	  iter++)
	{

	  assert((*iter)->Is("vcLoad"));
	  vcLoad* so = (vcLoad*) (*iter);
	  if(vcSystem::_enable_logging)
	  	so->vcSplitOperator::Print_VHDL_Logger(ofile);

	  if(ms == NULL)
	    ms = ((vcLoad*) so)->Get_Memory_Space();
	  else
	    assert(ms == ((vcLoad*) so)->Get_Memory_Space());

	  elements.push_back(so->Get_VHDL_Id());
	  dpe_elements.push_back(so);

	  so->Append_Inwires(inwires);
	  so->Append_Inwire_Buffering(inwire_buffering);

	  so->Append_Outwires(outwires);
	  so->Append_Outwire_Buffering(outwire_buffering);
   
	  reqL.push_back(so->Get_Req(0));
	  ackL.push_back(so->Get_Ack(0));
	  reqR.push_back(so->Get_Req(1));
	  ackR.push_back(so->Get_Ack(1));

	  so->Append_Guard(guard_wires,guard_complements);
	}

      assert(ms != NULL);

      // address, data
      int addr_width = ((vcLoadStore*)(*_compatible_load_groups[idx].begin()))->Get_Address()->Get_Size();
      int data_width = ((vcLoadStore*)(*_compatible_load_groups[idx].begin()))->Get_Data()->Get_Size();
      
      // tag-length
      int tag_length = ms->Get_Tag_Length();

      // total in-width 
      int in_width = 0;
      for(int u = 0; u < inwires.size(); u++)
	{
	  in_width += inwires[u]->Get_Size();
	}

      // total out-width..
      int out_width = 0;
      for(int u = 0; u < outwires.size(); u++)
	{
	  out_width += outwires[u]->Get_Size();
	}

      // pipelined case: slots per output at the operator.
      vector<int> slot_demands;
      int mbuf_size = this->Generate_Pipeline_Slot_Demands(dpe_elements, slot_demands);

      // VHDL code for this shared group
      ofile << "-- shared load operator group (" << idx << ") : " ;
      for(int u = 0; u < elements.size(); u++)
	ofile << elements[u] << " ";
      ofile << endl;

      // string buffering_string;
      // this->Generate_Buffering_Constant_Declaration(dpe_elements, buffering_string);

      string input_buffering_string;
      int maxipb = this->Generate_Buffering_String("inBUFs", inwire_buffering, input_buffering_string);

      string output_buffering_string;
      int maxopb = this->Generate_Buffering_String("outBUFs", outwire_buffering, output_buffering_string);

      string guard_flags;
      string guard_buffering;
      Generate_Guard_Constants(guard_buffering, guard_flags, dpe_elements, guard_wires);

      string group_name = "LoadGroup" + IntToStr(idx);
      // make a block
      ofile << group_name << ": Block -- {" << endl;
      // in and out data.
      ofile << "signal data_in: std_logic_vector(" << in_width-1 << " downto 0);" << endl;
      ofile << "signal data_out: std_logic_vector(" << out_width-1 << " downto 0);" << endl;
      // in and out acks.
      ofile << "signal reqR, ackR, reqL, ackL : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqR_unguarded, ackR_unguarded, reqL_unguarded, ackL_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqL_unregulated, ackL_unregulated: BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal guard_vector : std_logic_vector( " << num_reqs-1 << " downto 0);" << endl;
      //ofile << buffering_string  <<  endl;
      ofile << input_buffering_string  <<  endl;
      ofile << output_buffering_string  <<  endl;
      ofile << guard_flags << endl;
      ofile << guard_buffering << endl;
      ofile << "-- }\n begin -- {" << endl;

      // logging!
      if(vcSystem::_enable_logging)
	{
	  ofile << "-- logging on!" << endl;
	  for(int u = 0; u < inwires.size(); u++)
	    {

	      vcTransition* lrr = reqL[u];
	      vcTransition* lra = ackL[u];
	      vcTransition* lcr = reqR[u];
	      vcTransition* lca = ackR[u];
	      vcWire* iw = inwires[u];
	      vcWire* ow = outwires[u];

	      ofile << "LogMemRead(clk, reset, global_clock_cycle_count,-- { "  << endl
		    << lrr->Get_CP_To_DP_Symbol() << "," << endl
		    << lra->Get_DP_To_CP_Symbol() << "," << endl
		    << lcr->Get_CP_To_DP_Symbol() << "," << endl
		    << lca->Get_DP_To_CP_Symbol() << "," << endl
		    << "\"" << elements[u] << "\"," << endl
		    << "\"" << ms->Get_VHDL_Id() << "\" ," << endl
		    << ow->Get_VHDL_Signal_Id() << "," << endl
		    << iw->Get_VHDL_Signal_Id() << "," << endl
		    << "\"" << ow->Get_VHDL_Signal_Id() << "\"," << endl
		    << "\"" << iw->Get_VHDL_Signal_Id() << "\" -- } " << endl
		    << ");" << endl;
	    }
	}


      Print_VHDL_Concatenate_Req("reqL_unguarded",reqL,ofile);
      Print_VHDL_Disconcatenate_Ack("ackL_unguarded",ackL,ofile);
      Print_VHDL_Concatenate_Req("reqR_unguarded",reqR,ofile);
      Print_VHDL_Disconcatenate_Ack("ackR_unguarded",ackR,ofile);

      // prepare guard vector.
      Print_VHDL_Guard_Concatenation(num_reqs, "guard_vector", guard_wires, guard_complements, ofile);

      this->Print_VHDL_Regulator_Instance(group_name + "_accessRegulator", num_reqs, "reqL_unregulated", "ackL_unregulated", "reqL", "ackL", "reqR", "ackR", dpe_elements, ofile);

      /*
	 this->Print_VHDL_Guard_Instance("gI0", num_reqs,"guard_vector", "reqL_unguarded", "ackL_unguarded", "reqL_unregulated", "ackL_unregulated",
	 false,  ofile);
	 this->Print_VHDL_Guard_Instance("gI1", num_reqs,"guard_vector", "reqR_unguarded", "ackR_unguarded", "reqR", "ackR", true, ofile);
	 */
      Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
		      "reqL_unguarded", "ackL_unguarded",
		      "reqL_unregulated", "ackL_unregulated",
		      "reqR_unguarded", "ackR_unguarded",
		      "reqR", "ackR", ofile);

      // concatenate data_in
      Print_VHDL_Concatenation(string("data_in"), inwires,ofile);

      // disconcatenate data_out
      Print_VHDL_Disconcatenation(string("data_out"), out_width, outwires,ofile);

      vcModule* m = this->Get_Parent();

      // now the operator instances 
      string name = '"' + group_name + '"';

      ofile << "LoadReq: LoadReqSharedWithInputBuffers -- {" << endl;
      ofile << "generic map ( name => " << name << ", addr_width => " << addr_width << "," << endl
	      << "  num_reqs => " << num_reqs << "," << endl
	      << "  tag_length => " << tag_length << "," << endl
	      << "  time_stamp_width => " << ms->Calculate_Time_Stamp_Width() << "," << endl
	      << " min_clock_period => " << (vcSystem::_min_clock_period_flag ? "true" : "false") << "," << endl
	      << " input_buffering => inBUFs, " << endl
	      << "  no_arbitration => " << no_arb_string << ")" << endl;
      ofile << "port map ( -- { \n reqL => reqL " << ", " <<  endl
	      << "    ackL => ackL " << ", " <<  endl
	      << "    dataL => data_in, " << endl
	      << "    mreq => " 
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m,"load", "lr_req", idx)  << "," << endl
	      << "    mack => " 
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m, "load", "lr_ack", idx)  << "," << endl
	      << "    maddr => " 
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m, "load", "lr_addr",idx) << "," << endl
	      << "    mtag => "  
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m , "load", "lr_tag",idx) << "," << endl
	      << "  clk => clk, reset => reset -- }\n); -- }" << endl;

      ofile << "LoadComplete: LoadCompleteShared -- {" << endl;
      ofile << "generic map ( name => " << '"' <<  group_name 
	      << " load-complete "  << '"' << "," << endl
	      << " data_width => " << data_width << "," << endl
	      << "  num_reqs => " << num_reqs << "," << endl
	      << "  tag_length => " << tag_length << "," << endl
	      << "  detailed_buffering_per_output => outBUFs, " << endl
	      << "  no_arbitration => " << no_arb_string << ")" << endl;
      ofile << "port map ( -- {\n reqR => reqR " << ", " <<  endl
	      << "    ackR => ackR " << ", " <<  endl
	      << "    dataR => data_out, " << endl
	      << "    mreq => " 
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m,"load", "lc_req", idx)  
	      << "," << endl
	      << "    mack => " 
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m,"load", "lc_ack", idx)  
	      << "," << endl
	      << "    mdata => " 
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m,"load", "lc_data", idx)  << "," << endl
	      << "    mtag => "  
	      << ms->Get_VHDL_Memory_Interface_Port_Section(m, "load", "lc_tag", idx)  << "," << endl
	      << "  clk => clk, reset => reset -- }\n); -- }" << endl;
      ofile << "-- }\n end Block; -- load group " << idx << endl; // thats it..
    }
}

void vcDataPath::Print_VHDL_Store_Instances(ostream& ofile)
{ 
	string no_arb_string = (vcSystem::_min_area_flag ? "false" : "true");

	for(int idx = 0; idx < this->_compatible_store_groups.size(); idx++)
	{ // for each operator group.

		// number of requesters.
		int num_reqs = _compatible_store_groups[idx].size();

		vector<vcWire*> addrwires;
		vector<int> inwire_buffering;

		vector<vcWire*> datawires;
		vector<int> outwire_buffering;

		vector<vcTransition*> reqL;

		vector<vcTransition*> ackL;
		vector<vcTransition*> reqR;
		vector<vcTransition*> ackR;
		vector<vcWire*> guard_wires;
		vector<bool> guard_complements;

		// to keep track of the ids of the operators in this group.
		vector<string> elements;
		vector<vcDatapathElement*> dpe_elements;

      vcMemorySpace* ms = NULL;

      // ok. collect all the information..
      for(set<vcDatapathElement*>::iterator iter = _compatible_store_groups[idx].begin();
	  iter != _compatible_store_groups[idx].end();
	  iter++)
	{
	  assert((*iter)->Is("vcStore"));
	  vcStore* so = (vcStore*) (*iter);
	  if(vcSystem::_enable_logging)
	  	so->vcSplitOperator::Print_VHDL_Logger(ofile);

	  if(ms == NULL)
	    ms = so->Get_Memory_Space();
	  else
	    assert(ms == so->Get_Memory_Space());

	  elements.push_back(so->Get_VHDL_Id());
	  dpe_elements.push_back(so);

	  addrwires.push_back(so->Get_Address());
          inwire_buffering.push_back(so->Get_Input_Buffering(so->Get_Address()));

	  datawires.push_back(so->Get_Data());
          outwire_buffering.push_back(so->Get_Output_Buffering(so->Get_Data()));

	  reqL.push_back(so->Get_Req(0));
	  ackL.push_back(so->Get_Ack(0));
	  reqR.push_back(so->Get_Req(1));
	  ackR.push_back(so->Get_Ack(1));

	  so->Append_Guard(guard_wires,guard_complements);
	}
      assert(ms != NULL);

      // string buffering_string;
      // this->Generate_Buffering_Constant_Declaration(dpe_elements, buffering_string);


      // logging!
      if(vcSystem::_enable_logging)
	{
	  
	  ofile << "-- logging on!" << endl;
	  for(int u = 0; u < addrwires.size(); u++)
	    {

	      vcTransition* srr = reqL[u];
	      vcTransition* sra = ackL[u];
	      vcTransition* scr = reqR[u];
	      vcTransition* sca = ackR[u];

	      vcWire* aw = addrwires[u];
	      vcWire* dw = datawires[u];

	      
	      ofile << "LogMemWrite(clk, reset,global_clock_cycle_count,  -- { "  << endl
		    << srr->Get_CP_To_DP_Symbol() << "," << endl
		    << sra->Get_DP_To_CP_Symbol() << "," << endl
		    << scr->Get_CP_To_DP_Symbol() << "," << endl
		    << sca->Get_DP_To_CP_Symbol() << "," << endl
		    << "\"" << elements[u] << "\"," << endl
		    << "\"" << ms->Get_VHDL_Id() << "\" ," << endl
		    << dw->Get_VHDL_Signal_Id() << "," << endl
		    << aw->Get_VHDL_Signal_Id() << "," << endl
		    << "\"" << dw->Get_VHDL_Signal_Id() << "\"," << endl
		    << "\"" << aw->Get_VHDL_Signal_Id() << "\" -- } " << endl
		    << ");" << endl;
	    }
	}

      // address, data
      int addr_width = ((vcLoadStore*)(*_compatible_store_groups[idx].begin()))->Get_Address()->Get_Size();
      assert(addr_width == ms->Get_Address_Width());
      int data_width = ((vcLoadStore*)(*_compatible_store_groups[idx].begin()))->Get_Data()->Get_Size();
      assert(data_width == ms->Get_Word_Size());
      
      // tag-length
      int tag_length = ms->Get_Tag_Length();

      // total addr-width 
      int total_addr_width = addrwires.size() * addr_width;
      int total_data_width = datawires.size() * data_width;
      
      // VHDL code for this shared group
      ofile << "-- shared store operator group (" << idx << ") : " ;
      for(int u = 0; u < elements.size(); u++)
	ofile << elements[u] << " ";
      ofile << endl;


      string input_buffering_string;
      int maxipb = this->Generate_Buffering_String("inBUFs", inwire_buffering, input_buffering_string);

      string output_buffering_string;
      int maxopb = this->Generate_Buffering_String("outBUFs", outwire_buffering, output_buffering_string);

      string guard_flags;
      string guard_buffering;
      Generate_Guard_Constants(guard_buffering, guard_flags, dpe_elements, guard_wires);

      string group_name = "StoreGroup" + IntToStr(idx);

      // make a block
      ofile << group_name << ": Block -- {" << endl;
      // in and out data.
      ofile << "signal addr_in: std_logic_vector(" << total_addr_width-1 << " downto 0);" << endl;
      ofile << "signal data_in: std_logic_vector(" << total_data_width-1 << " downto 0);" << endl;
      // in and out acks.
      ofile << "signal reqR, ackR, reqL, ackL : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqR_unguarded, ackR_unguarded, reqL_unguarded, ackL_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqL_unregulated, ackL_unregulated : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal guard_vector : std_logic_vector( " << num_reqs-1 << " downto 0);" << endl;
      //ofile << buffering_string << endl;
      ofile << input_buffering_string  <<  endl;
      ofile << output_buffering_string  <<  endl;
      ofile << guard_flags << endl;
      ofile << guard_buffering << endl;

      ofile << "-- }\n begin -- {" << endl;

      Print_VHDL_Concatenate_Req("reqL_unguarded",reqL,ofile);
      Print_VHDL_Disconcatenate_Ack("ackL_unguarded",ackL,ofile);
      Print_VHDL_Concatenate_Req("reqR_unguarded",reqR,ofile);
      Print_VHDL_Disconcatenate_Ack("ackR_unguarded",ackR,ofile);

      // prepare guard vector..
      Print_VHDL_Guard_Concatenation(num_reqs, "guard_vector", guard_wires, guard_complements, ofile);


      this->Print_VHDL_Regulator_Instance(group_name + "_accessRegulator", num_reqs, "reqL_unregulated", "ackL_unregulated", "reqL", "ackL", "reqR", "ackR", dpe_elements, ofile);

	/*
      this->Print_VHDL_Guard_Instance("gI0", num_reqs,"guard_vector", "reqL_unguarded", "ackL_unguarded", "reqL_unregulated", "ackL_unregulated",
			false, ofile);
      this->Print_VHDL_Guard_Instance("gI1", num_reqs,"guard_vector", "reqR_unguarded", "ackR_unguarded", "reqR", "ackR", true, ofile);
	*/

      Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
		      "reqL_unguarded", "ackL_unguarded",
		      "reqL_unregulated", "ackL_unregulated",
		      "reqR_unguarded", "ackR_unguarded",
		      "reqR", "ackR", ofile);

      // concatenate addr_in and data_in
      Print_VHDL_Concatenation(string("addr_in"), addrwires,ofile);
      Print_VHDL_Concatenation(string("data_in"), datawires,ofile);

      vcModule* m = this->Get_Parent();

      string sr_name = '"' + group_name + " Req " + '"';
      // now the operator instances 
      ofile << "StoreReq: StoreReqSharedWithInputBuffers -- {" << endl;
      ofile << "generic map ( name => " << sr_name << ", addr_width => " << addr_width << "," << endl
	    << "  data_width => " << data_width << "," << endl
	    << "  num_reqs => " << num_reqs << "," << endl
	    << "  tag_length => " << tag_length << "," << endl
	    << "  time_stamp_width => " << ms->Calculate_Time_Stamp_Width() << "," << endl
	    << "  min_clock_period => " << (vcSystem::_min_clock_period_flag ? "true" : "false") << "," << endl
	    << "  input_buffering => inBUFs, " << endl
	    << "  no_arbitration => " << no_arb_string << ")" << endl;
      ofile << "port map (--{\n reqL => reqL " << ", " <<  endl
	    << "    ackL => ackL " << ", " <<  endl
	    << "    addr => addr_in, " << endl
	    << "    data => data_in, " << endl
	    << "    mreq => " 
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store", "sr_req", idx)  << "," << endl
	    << "    mack => " 
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store", "sr_ack", idx)  << "," << endl
	    << "    maddr => " 
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store","sr_addr",idx) << "," << endl
	    << "    mdata => " 
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store","sr_data",idx) << "," << endl
	    << "    mtag => "  
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store","sr_tag",idx) << "," << endl
	    << "  clk => clk, reset => reset -- }\n);--}" << endl;

      string sc_name = '"' + group_name + " Complete " + '"';
      ofile << "StoreComplete: StoreCompleteShared -- { " << endl;
      ofile << "generic map ( -- { "
	    << " name => " << sc_name  << "," << endl
	    << "  num_reqs => " << num_reqs << "," << endl
	    << "  detailed_buffering_per_output => outBUFs," << endl
	    << "  tag_length => " << tag_length << " -- } \n)" << endl;
      ofile << "port map ( -- { reqR => reqR " << ", " <<  endl
	    << "    ackR => ackR " << ", " <<  endl
	    << "    mreq => " 
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store", "sc_req", idx)  << "," << endl
	    << "    mack => " 
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store", "sc_ack", idx)  << "," << endl
	    << "    mtag => "  
	    << ms->Get_VHDL_Memory_Interface_Port_Section(m,"store", "sc_tag", idx)  << "," << endl
	    << "  clk => clk, reset => reset -- }); -- }" << endl;

      ofile << "-- }\n end Block; -- store group " << idx << endl; // thats it..
    }
}

void vcDataPath::Print_VHDL_Inport_Instances(ostream& ofile)
{

  string no_arb_string = (vcSystem::_min_area_flag ? "false" : "true");

  for(int idx = 0; idx < this->_compatible_inport_groups.size(); idx++)
    { // for each operator group.

      // number of requesters.
      int num_reqs = _compatible_inport_groups[idx].size();

      vector<vcTransition*> reqL;
      vector<vcTransition*> ackL;
      vector<vcTransition*> reqR;
      vector<vcTransition*> ackR;

      vector<vcWire*> outwires;
      vector<int> outwire_buffering;

      vector<vcWire*> guards;
      vector<vcDatapathElement*> dpe_elements;
      vector<bool> guard_complements;

      // to keep track of the ids of the operators in this group.
      vector<string> elements;

 
    

      vcPipe* p = NULL;
      int data_width = -1;

      // ok. collect all the information..
      for(set<vcDatapathElement*>::iterator iter = _compatible_inport_groups[idx].begin();
	  iter != _compatible_inport_groups[idx].end();
	  iter++)
	{

	  assert((*iter)->Is("vcInport"));
	  vcInport* so = (vcInport*) (*iter);
	  if(vcSystem::_enable_logging)
	  	so->vcOperator::Print_VHDL_Logger(ofile);

	  if(p == NULL)
	    p = ((vcInport*) so)->Get_Pipe();
	  else
	    assert(p == ((vcInport*) so)->Get_Pipe());

	  if(data_width < 0)
	    data_width = so->Get_Data()->Get_Size();
	  else
	    assert(data_width == so->Get_Data()->Get_Size());

	  elements.push_back(so->Get_VHDL_Id());
	  dpe_elements.push_back(so);
	  so->Append_Outwires(outwires);
          so->Append_Outwire_Buffering(outwire_buffering);

	  reqL.push_back(so->Get_Req(0));
	  ackL.push_back(so->Get_Ack(0));
	  reqR.push_back(so->Get_Req(1));
	  ackR.push_back(so->Get_Ack(1));

 	  so->Append_Guard(guards, guard_complements);
	}
      assert(p != NULL);
      assert(data_width > 0);

      // total out-width..
      int out_width = 0;
      for(int u = 0; u < outwires.size(); u++)
	{
	  out_width += outwires[u]->Get_Size();
	}

      

      // VHDL code for this shared group
      ofile << "-- shared inport operator group (" << idx << ") : " ;
      for(int u = 0; u < elements.size(); u++)
	ofile << elements[u] << " ";
      ofile << endl;

      string output_buffering_string;
      int maxopb = this->Generate_Buffering_String("outBUFs", outwire_buffering, output_buffering_string);

      string guard_flags;
      string guard_buffering;
      Generate_Guard_Constants(guard_buffering, guard_flags, dpe_elements, guards);

      // make a block
      ofile << "InportGroup" << idx << ": Block -- {" << endl;
      // in and out data.
      ofile << "signal data_out: std_logic_vector(" << out_width-1 << " downto 0);" << endl;
      // in and out acks.
      ofile << "signal reqL, ackL, reqR, ackR : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqL_unguarded, ackL_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqR_unguarded, ackR_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal guard_vector : std_logic_vector( " << num_reqs-1 << " downto 0);" << endl;

      ofile << output_buffering_string << endl;
      ofile << guard_flags << endl;
      ofile << guard_buffering << endl;

      ofile << "-- }\n begin -- {" << endl;

      // logging!
      if(vcSystem::_enable_logging)
	{
	  ofile << "-- logging on!" << endl;
	  ofile << " process(clk)  begin -- {" 
		<< " if clk'event and clk = '1' then -- {" << endl;
	  int lindex = out_width-1;
	  for(int u = 0; u < outwires.size(); u++)
	    {

	      vcTransition* aw = ackR[u];
	      vcWire* ow = outwires[u];
	      
	      ofile << " if " << aw->Get_DP_To_CP_Symbol() << " then -- {" << endl;
	      ofile << " assert false report \" ReadPipe " 
		    <<  p->Get_VHDL_Id() 
		    << " to wire " << ow->Get_VHDL_Signal_Id() << " value=\" " 
		    << " & "
		    << " convert_slv_to_hex_string(data_out(" 
		    << lindex << " downto " << (lindex - (data_width-1)) << ")) "
		    << " severity note; --}" << endl;
	      ofile << " end if;" << endl;
	      lindex -= data_width;
	    }
	  ofile << " --} " << endl << "end if;" << endl;
	  ofile << "-- } " << endl << " end process;" << endl;
	}

      // guard related stuff.
      Print_VHDL_Concatenate_Req("reqL_unguarded",reqL,ofile);
      Print_VHDL_Disconcatenate_Ack("ackL_unguarded",ackL,ofile);
      Print_VHDL_Concatenate_Req("reqR_unguarded",reqR,ofile);
      Print_VHDL_Disconcatenate_Ack("ackR_unguarded",ackR,ofile);
      Print_VHDL_Guard_Concatenation(num_reqs, "guard_vector", guards, guard_complements, ofile);

	/*
      this->Print_VHDL_Guard_Instance("gI", num_reqs,"guard_vector", "reqL_unguarded", "ackL_unguarded", "reqL", "ackL", false, ofile);
      this->Print_VHDL_Guard_Instance("gO", num_reqs,"guard_vector", "reqR_unguarded", "ackR_unguarded", "reqR", "ackR", true, ofile);
	*/
      Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
		      "reqL_unguarded", "ackL_unguarded",
		      "reqL", "ackL",
		      "reqR_unguarded", "ackR_unguarded",
		      "reqR", "ackR", ofile);

      // disconcatenate data_out
      Print_VHDL_Disconcatenation(string("data_out"), out_width, outwires,ofile);

      // now the operator instances 
      string group_name = p->Get_VHDL_Id() + "_read_" + IntToStr(idx);
      string name = '"' + group_name + '"';
      ofile << group_name << ": InputPortFullRate -- { " << endl;
      ofile << "generic map ( name => " << name << ", data_width => " << data_width << ","
	    << "  num_reqs => " << num_reqs << ","
	    << "  output_buffering => outBUFs, "
	    << "  no_arbitration => " << no_arb_string << ")" << endl;
      ofile << "port map (-- {\n sample_req => reqL " << ", " <<  endl
	    << "    sample_ack => ackL" << ", " <<  endl
	    << "    update_req => reqR" << ", " <<  endl
	    << "    update_ack => ackR" << ", " <<  endl
	    << "    data => data_out, " << endl
	    << "    oreq => " 
	    << this->Get_VHDL_IOport_Interface_Port_Section(p, "in" , "read_req", idx)  << "," << endl
	    << "    oack => " 
	    << this->Get_VHDL_IOport_Interface_Port_Section(p,"in",  "read_ack", idx)  << "," << endl
	    << "    odata => " 
	    << this->Get_VHDL_IOport_Interface_Port_Section(p,"in", "read_data", idx)  << "," << endl
	    << "  clk => clk, reset => reset -- }\n ); -- }" << endl;
      ofile << "-- }\n end Block; -- inport group " << idx << endl; // thats it..
    }
}


void vcDataPath::Print_VHDL_Outport_Instances(ostream& ofile)
{ 
  string no_arb_string = (vcSystem::_min_area_flag ? "false" : "true");

  for(int idx = 0; idx < this->_compatible_outport_groups.size(); idx++)
    { // for each operator group.

      // number of requesters.
      int num_reqs = _compatible_outport_groups[idx].size();

      vector<vcTransition*> sample_req;
      vector<vcTransition*> sample_ack;
      vector<vcTransition*> update_req;
      vector<vcTransition*> update_ack;

      vector<vcWire*> inwires;
      vector<int> inwire_buffering;

      vector<vcWire*> guards;
      vector<bool> guard_complements;

      vector<vcDatapathElement*> dpe_elements;

      // to keep track of the ids of the operators in this group.
      vector<string> elements;

      vcPipe* p = NULL;
      int data_width = -1;

      // ok. collect all the information..
      for(set<vcDatapathElement*>::iterator iter = _compatible_outport_groups[idx].begin();
	  iter != _compatible_outport_groups[idx].end();
	  iter++)
	{

	  assert((*iter)->Is("vcOutport"));
	  vcOutport* so = (vcOutport*) (*iter);
	  if(vcSystem::_enable_logging)
	  	so->vcOperator::Print_VHDL_Logger(ofile);

	  if(p == NULL)
	    p = ((vcOutport*) so)->Get_Pipe();
	  else
	    assert(p == ((vcOutport*) so)->Get_Pipe());

	  if(data_width < 0)
	    data_width = so->Get_Data()->Get_Size();
	  else
	    assert(data_width == so->Get_Data()->Get_Size());

	  elements.push_back(so->Get_VHDL_Id());
	  dpe_elements.push_back(so);

	  so->Append_Inwires(inwires);
	  so->Append_Inwire_Buffering(inwire_buffering);

	  sample_req.push_back(so->Get_Req(0));
	  sample_ack.push_back(so->Get_Ack(0));

	  update_req.push_back(so->Get_Req(1));
	  update_ack.push_back(so->Get_Ack(1));

 	  so->Append_Guard(guards, guard_complements);
	}
      assert(p != NULL);
      assert(data_width > 0);

      int in_width = 0;
      for(int u = 0; u < inwires.size(); u++)
	{
	  in_width += inwires[u]->Get_Size();
	}

      // logging!
      if(vcSystem::_enable_logging)
	{
	  
	  ofile << "-- logging on!" << endl;
	  ofile << " process(clk)  begin -- {" 
		<< " if clk'event and clk = '1' then -- {" << endl;
	  for(int u = 0; u < inwires.size(); u++)
	    {

	      vcTransition* aw = update_ack[u];
	      vcWire* iw = inwires[u];
	      
	      ofile << " if " << aw->Get_DP_To_CP_Symbol() << " then -- {" << endl;
	      ofile << " assert false report \" WritePipe " 
		    <<  p->Get_VHDL_Id() 
		    << " from wire " << iw->Get_VHDL_Signal_Id() << " value=\" " 
		    << " & "
		    << " convert_slv_to_hex_string(" << iw->Get_VHDL_Signal_Id() << ")"
		    << " severity note; --}" << endl;
	      ofile << " end if;" << endl;
	    }
	  ofile << " --} " << endl << "end if;" << endl;
	  ofile << "-- } " << endl << " end process;" << endl;
	}


      string input_buffering_string;
      int max_inbuf = this->Generate_Buffering_String("inBUFs", inwire_buffering, input_buffering_string);

      string guard_flags;
      string guard_buffering;
      Generate_Guard_Constants(guard_buffering, guard_flags, dpe_elements, guards);

      // VHDL code for this shared group
      ofile << "-- shared outport operator group (" << idx << ") : " ;
      for(int u = 0; u < elements.size(); u++)
	ofile << elements[u] << " ";
      ofile << endl;


      // make a block
      ofile << "OutportGroup" << idx << ": Block -- { " << endl;
      // in and out data.
      ofile << "signal data_in: std_logic_vector(" << in_width-1 << " downto 0);" << endl;
      // in and out acks.
      ofile << "signal sample_req, sample_ack : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal update_req, update_ack : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal sample_req_unguarded, sample_ack_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal update_req_unguarded, update_ack_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;

      ofile << "signal guard_vector : std_logic_vector( " << num_reqs-1 << " downto 0);" << endl;

      ofile << input_buffering_string << endl;
      ofile << guard_flags << endl;
      ofile << guard_buffering << endl;
      ofile << "-- }\n begin -- {" << endl;

      // guard related stuff.
      Print_VHDL_Concatenate_Req("sample_req_unguarded",sample_req,ofile);
      Print_VHDL_Disconcatenate_Ack("sample_ack_unguarded",sample_ack,ofile);
      Print_VHDL_Concatenate_Req("update_req_unguarded",update_req,ofile);
      Print_VHDL_Disconcatenate_Ack("update_ack_unguarded",update_ack,ofile);
      Print_VHDL_Guard_Concatenation(num_reqs, "guard_vector", guards, guard_complements, ofile);
	/*
      this->Print_VHDL_Guard_Instance("gI0", num_reqs,"guard_vector", "sample_req_unguarded", "sample_ack_unguarded", "sample_req", "sample_ack", false, ofile);
      this->Print_VHDL_Guard_Instance("gI1", num_reqs,"guard_vector", "update_req_unguarded", "update_ack_unguarded", "update_req", "update_ack", true, ofile);
	*/
      Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
		      "sample_req_unguarded", "sample_ack_unguarded",
		      "sample_req", "sample_ack",
		      "update_req_unguarded", "update_ack_unguarded",
		      "update_req", "update_ack", ofile);

      // concatenate data_in
      Print_VHDL_Concatenation(string("data_in"), inwires,ofile);

      // now the operator instances 
      string group_name = p->Get_VHDL_Id() + "_write_" + IntToStr(idx);
      string name = '"' + p->Get_VHDL_Id() + '"';
      ofile << group_name << ": OutputPortFullRate -- { " << endl;
      ofile << "generic map ( name => " << name << ", data_width => " << data_width << ","
	    << " num_reqs => " << num_reqs << ","
	    << " input_buffering => inBUFs," 
	    << " no_arbitration => " << no_arb_string << ")" << endl;
      ofile << "port map (--{\n sample_req => sample_req " << ", " <<  endl
	    << "    sample_ack => sample_ack " << ", " <<  endl
	    << "    update_req => update_req " << ", " <<  endl
	    << "    update_ack => update_ack " << ", " <<  endl
	    << "    data => data_in, " << endl
	    << "    oreq => " 
	    << this->Get_VHDL_IOport_Interface_Port_Section(p,"out" , "write_req", idx)  << "," << endl
	    << "    oack => " 
	    << this->Get_VHDL_IOport_Interface_Port_Section(p,"out",  "write_ack", idx)  << "," << endl
	    << "    odata => " 
	    << this->Get_VHDL_IOport_Interface_Port_Section(p,"out", "write_data", idx)  << "," << endl
	    << "  clk => clk, reset => reset -- }\n);-- }" << endl;
      ofile << "-- }\n end Block; -- outport group " << idx << endl; // thats it..
    }
}


string vcDataPath::Get_VHDL_IOport_Interface_Port_Name(string pipe_id, string pid)
{
  return(this->Get_Parent()->Get_Parent()->Get_VHDL_Pipe_Interface_Port_Name(pipe_id,pid));
}

string vcDataPath::Get_VHDL_IOport_Interface_Port_Section(vcPipe* p, 
							  string in_or_out,
							  string pid,
							  int idx)
{

  string pipe_id = p->Get_Id();
  string port_name = p->Get_VHDL_Pipe_Interface_Port_Name(pid);

  vcModule* m = this->Get_Parent();
  vcSystem* sys = m->Get_Parent();

  map<vcPipe*,vector<int> >::iterator iter;
  if(in_or_out == "in")
    {
      iter = _inport_group_map.find(p);
      assert(iter != _inport_group_map.end());
    }
  else
    {
      iter = _outport_group_map.find(p);
      assert(iter != _outport_group_map.end());
    }

  int down_index;
  // left to right 
  for(int index = 0; index < (*iter).second.size(); index++)
    {
      down_index = ((*iter).second.size()-1) - index; // position from left.
      if(idx == (*iter).second[index])
	break;
      if(index == (*iter).second.size() - 1)
	assert(0);
    }


  int pipe_width = p->Get_Width();

  if((pid.find("req") != string::npos) || (pid.find("ack") != string::npos))
    return(port_name +  "(" + IntToStr(down_index) + ")");
  else if(pid.find("data") != string::npos)
    return(port_name + "(" 
	   + IntToStr(((down_index+1)*pipe_width)-1) + " downto "
	   + IntToStr(down_index*pipe_width) + ")");
  else
    assert(0); // fatal
}


void vcDataPath::Print_VHDL_Call_Instances(ostream& ofile)
{ 

  string no_arb_string = (vcSystem::_min_area_flag ? "false" : "true");

  // print LoadReqShared instance and LoadCompleteShared instance.
  for(int idx = 0; idx < this->_compatible_call_groups.size(); idx++)
    { // for each operator group.

      // number of requesters.
      int num_reqs = _compatible_call_groups[idx].size();

      // to collect inwires, outwires and reqs/acks.
      vector<vcWire*> inwires;
      vector<int> inwire_buffering;

      vector<vcTransition*> reqL;
      vector<vcTransition*> ackL;
      vector<vcTransition*> reqR;
      vector<vcTransition*> ackR;

      vector<vcWire*> outwires;
      vector<int> outwire_buffering;

      vector<vcWire*> guard_wires;
      vector<bool> guard_complements;

      // to keep track of the ids of the operators in this group.
      vector<string> elements;
      vector<vcDatapathElement*> dpe_elements;

      vcModule* called_module = NULL;

      // ok. collect all the information..
      for(set<vcDatapathElement*>::iterator iter = _compatible_call_groups[idx].begin();
	  iter != _compatible_call_groups[idx].end();
	  iter++)
	{

	  assert((*iter)->Is("vcCall"));
	  vcCall* so = (vcCall*) (*iter);
	  if(vcSystem::_enable_logging)
	  	so->vcSplitOperator::Print_VHDL_Logger(ofile);

	  if(called_module == NULL)
	    called_module = so->Get_Called_Module();
	  else
	    assert(called_module == so->Get_Called_Module());

	  elements.push_back(so->Get_VHDL_Id());
	  dpe_elements.push_back(so);

	  so->Append_Inwires(inwires);
          so->Append_Inwire_Buffering(inwire_buffering);

	  so->Append_Outwires(outwires);
          so->Append_Outwire_Buffering(outwire_buffering);

	  reqL.push_back(so->Get_Req(0));
	  ackL.push_back(so->Get_Ack(0));
	  reqR.push_back(so->Get_Req(1));
	  ackR.push_back(so->Get_Ack(1));

	  so->Append_Guard(guard_wires,guard_complements);
	}
      assert(called_module != NULL);


      string input_buffering_string;
      int max_inbuf = this->Generate_Buffering_String("inBUFs", inwire_buffering, input_buffering_string);

      bool use_out_buffering = ((called_module->Get_Out_Arg_Width() > 0) ? true : false);
      string output_buffering_string;
      int max_outbuf;

	// special case.. hack alert.
      if(use_out_buffering)
      	max_outbuf  = this->Generate_Buffering_String("outBUFs", outwire_buffering, output_buffering_string);
      else
	output_buffering_string =  "constant outBUFs: IntegerArray(" + IntToStr(num_reqs-1) + " downto 0) := (others => 1);";


      string guard_flags;
      string guard_buffering;
      Generate_Guard_Constants(guard_buffering, guard_flags, dpe_elements, guard_wires);

      int tag_length = called_module->Get_Caller_Tag_Length();

      // total in-width 
      int in_width = 0;
      for(int u = 0; u < inwires.size(); u++)
	{
	  in_width += inwires[u]->Get_Size();
	}
      assert(in_width == called_module->Get_In_Arg_Width() * num_reqs);

      // total out-width..
      int out_width = 0;
      for(int u = 0; u < outwires.size(); u++)
	{
	  out_width += outwires[u]->Get_Size();
	}
      assert(out_width == called_module->Get_Out_Arg_Width() * num_reqs);

      // VHDL code for this shared group
      ofile << "-- shared call operator group (" << idx << ") : " ;
      for(int u = 0; u < elements.size(); u++)
	ofile << elements[u] << " ";
      ofile << endl;


      // make a block
      string group_name = called_module->Get_VHDL_Id() + "_call_group_" + IntToStr(idx);
      ofile << group_name << ": Block -- {" << endl;
      // in and out data.
      if(called_module->Get_In_Arg_Width() > 0)
	ofile << "signal data_in: std_logic_vector(" << in_width-1 << " downto 0);" << endl;
      if(called_module->Get_Out_Arg_Width() > 0)
	ofile << "signal data_out: std_logic_vector(" << out_width-1 << " downto 0);" << endl;
      // in and out acks.
      ofile << "signal reqR, ackR, reqL, ackL : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqR_unguarded, ackR_unguarded, reqL_unguarded, ackL_unguarded : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal reqL_unregulated, ackL_unregulated : BooleanArray( " << num_reqs-1 << " downto 0);" << endl;
      ofile << "signal guard_vector : std_logic_vector( " << num_reqs-1 << " downto 0);" << endl;

      // ofile << buffering_string << endl;
      ofile << input_buffering_string << endl;
      ofile << output_buffering_string << endl;
      ofile << guard_flags << endl;
      ofile << guard_buffering << endl;
      ofile << "-- }\n begin -- {" << endl;

      Print_VHDL_Concatenate_Req("reqL_unguarded",reqL,ofile);
      Print_VHDL_Disconcatenate_Ack("ackL_unguarded",ackL,ofile);
      Print_VHDL_Concatenate_Req("reqR_unguarded",reqR,ofile);
      Print_VHDL_Disconcatenate_Ack("ackR_unguarded",ackR,ofile);
	
      // prepare guard vector.
      Print_VHDL_Guard_Concatenation(num_reqs, "guard_vector", guard_wires, guard_complements, ofile);


      this->Print_VHDL_Regulator_Instance(group_name + "_accessRegulator", num_reqs,"reqL_unregulated", "ackL_unregulated", "reqL", "ackL", "reqR", "ackR", dpe_elements,  ofile);

	/*
      this->Print_VHDL_Guard_Instance("gI0", num_reqs,"guard_vector", "reqL_unguarded", "ackL_unguarded", "reqL_unregulated", "ackL_unregulated",
			false, ofile);
      this->Print_VHDL_Guard_Instance("gI1", num_reqs,"guard_vector", "reqR_unguarded", "ackR_unguarded", "reqR", "ackR", true, ofile);
	*/
      Print_VHDL_Guard_Instance("gI",num_reqs,"guardBuffering","guardFlags","guard_vector",
		      "reqL_unguarded", "ackL_unguarded",
		      "reqL_unregulated", "ackL_unregulated",
		      "reqR_unguarded", "ackR_unguarded",
		      "reqR", "ackR", ofile);

      // concatenate data_in
      if(called_module->Get_In_Arg_Width() > 0)
	Print_VHDL_Concatenation(string("data_in"), inwires,ofile);

      // disconcatenate data_out
      if(called_module->Get_Out_Arg_Width() > 0)
	Print_VHDL_Disconcatenation(string("data_out"), out_width, outwires,ofile);

      vcModule* m = this->Get_Parent();
      // now the operator instances 
      string imux_name = ((called_module->Get_In_Arg_Width() > 0) ?
			  "InputMuxWithBuffering" : "InputMuxBaseNoData");
      bool use_in_buffering =( (called_module->Get_In_Arg_Width() > 0) ? true : false);

      ofile << "CallReq: " << imux_name << " -- {" << endl;
      ofile << "generic map ( ";

      if(called_module->Get_In_Arg_Width() > 0)
	{
	  ofile << " iwidth => " << in_width << "," << endl
		<< " owidth => " << in_width/num_reqs << "," << endl;
	}

      if(use_in_buffering)
      {
	  ofile << " name => " << '"' << imux_name << '"' << ","  << endl;
	  ofile << " buffering => inBUFs,"  << endl;
      }
	
      ofile << " twidth => " << tag_length << "," << endl
	    << " nreqs => " << num_reqs << "," << endl;


      if(called_module->Get_In_Arg_Width() > 0)
        ofile << " registered_output => "
	      << (vcSystem::_min_clock_period_flag ? "true" : "false") << "," << endl;

 
      ofile   << "  no_arbitration => " << no_arb_string << ")" << endl;
      ofile << "port map ( -- { \n reqL => reqL " << ", " <<  endl
	    << "    ackL => ackL " << ", " <<  endl;

      if(called_module->Get_In_Arg_Width() > 0)
	ofile << "    dataL => data_in, " << endl;

      ofile << "    reqR => " 
	    << called_module->Get_VHDL_Call_Interface_Port_Section(m,"call", "call_reqs", idx)  << "," << endl
	    << "    ackR => " 
	    << called_module->Get_VHDL_Call_Interface_Port_Section(m, "call", "call_acks", idx)  << "," << endl;

      if(called_module->Get_In_Arg_Width() > 0)
	ofile << "    dataR => " 
	      << called_module->Get_VHDL_Call_Interface_Port_Section(m, "call", "call_data",idx) << "," << endl;

      
      ofile << "    tagR => "  
	    << called_module->Get_VHDL_Call_Interface_Port_Section(m , "call", "call_tag",idx) 
	    << "," << endl;

      ofile << "  clk => clk, reset => reset -- }\n); -- }" << endl;


      string omux_name = ((called_module->Get_Out_Arg_Width() > 0) ?
			  "OutputDemuxBaseWithBuffering" : "OutputDemuxBaseNoData");

      ofile << "CallComplete: " << omux_name << " -- {" << endl;
      ofile << "generic map ( -- {" << endl;
      if(called_module->Get_Out_Arg_Width() > 0)
	{
	  ofile << "iwidth => " << out_width/num_reqs << "," << endl
		<< " owidth => " << out_width << "," << endl; 
	}
      ofile << " detailed_buffering_per_output => outBUFs, " << endl;

      ofile << " twidth => " << tag_length << "," << endl
	    << " name => " << '"' << omux_name << '"' << "," << endl
	    << " nreqs => " << num_reqs << ") -- }" << endl;
      ofile << "port map ( -- {\n reqR => reqR " << ", " <<  endl
	    << "    ackR => ackR " << ", " <<  endl;

      if(called_module->Get_Out_Arg_Width() > 0)
	ofile << "    dataR => data_out, " << endl;

      ofile << "    reqL => " 
	    << called_module->Get_VHDL_Call_Interface_Port_Section(m,"return", "return_acks", idx)  << ", -- cross-over" << endl
	    << "    ackL => " 
	    << called_module->Get_VHDL_Call_Interface_Port_Section(m,"return", "return_reqs", idx)  << ", -- cross-over" << endl;

      if(called_module->Get_Out_Arg_Width() > 0)
	{
	  ofile << "    dataL => " 
		<< called_module->Get_VHDL_Call_Interface_Port_Section(m,"return", "return_data", idx)  << "," << endl;
	}

      ofile << "    tagL => "  
	    << called_module->Get_VHDL_Call_Interface_Port_Section(m, "return", "return_tag", idx)  << "," << endl
	    << "  clk => clk," << endl
	    << " reset => reset -- }\n); -- }" << endl;
      ofile << "-- }\n end Block; -- call group " << idx << endl; // thats it..
    }
}


string vcDataPath::Print_VHDL_Memory_Interface_Port_Map(string comma, ostream& ofile)
{

  // in progress
  set<vcMemorySpace*,vcRoot_Compare> ms_set;
  vcMemorySpace* ms;
  vcModule* parent_module = this->Get_Parent();

  // first the loads
  for(int idx = 0; idx < _compatible_load_groups.size(); idx++)
    {
      ms = ((vcLoad*) (*(_compatible_load_groups[idx].begin())))->Get_Memory_Space();
      ms_set.insert(ms);
    }

  for(set<vcMemorySpace*,vcRoot_Compare>::iterator iter = ms_set.begin();
      iter != ms_set.end();
      iter++)
    {
      ms = (*iter);

      if(ms->Get_Scope() == NULL) // only if ms is at the system level
	{
	  // if ms is not inside this module, then print a port map..
	  int hindex, lindex;
	  if(ms->Get_Caller_Module_Section(parent_module,"load",hindex,lindex))
	    {
	      ofile << comma << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_req") << " => " <<
		ms->Get_Aggregate_Section("lr_req", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_ack") << " => " <<
		ms->Get_Aggregate_Section("lr_ack", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_addr") << " => " << 
		ms->Get_Aggregate_Section("lr_addr", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lr_tag") << " => " << 
		ms->Get_Aggregate_Section("lr_tag", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_req") << " => " << 
		ms->Get_Aggregate_Section("lc_req", hindex, lindex)<< "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_ack") << " => " << 
		ms->Get_Aggregate_Section("lc_ack", hindex, lindex)<< "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_data") << " => " << 
		ms->Get_Aggregate_Section("lc_data", hindex, lindex)<< "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("lc_tag") << " => " << 
		ms->Get_Aggregate_Section("lc_tag", hindex, lindex);
	      comma = ",";
	    }
	}
    }
  

  ms_set.clear();

  // now the stores
  for(int idx = 0; idx < _compatible_store_groups.size(); idx++)
    {
      ms = ((vcLoad*) (*(_compatible_store_groups[idx].begin())))->Get_Memory_Space();
      ms_set.insert(ms);
    }

  for(set<vcMemorySpace*,vcRoot_Compare>::iterator iter = ms_set.begin();
      iter != ms_set.end();
      iter++)
    {
      ms = (*iter);

      if(ms->Get_Scope() == NULL) // only if ms is at the system level
	{
	  int hindex, lindex;
	  if(ms->Get_Caller_Module_Section(parent_module,"store",hindex,lindex))
	    {
	      ofile << comma << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_req") << " => " <<
		ms->Get_Aggregate_Section("sr_req", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_ack") << " => " <<
		ms->Get_Aggregate_Section("sr_ack", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_addr") << " => " << 
		ms->Get_Aggregate_Section("sr_addr", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_data") << " => " << 
		ms->Get_Aggregate_Section("sr_data", hindex, lindex)<< "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sr_tag") << " => " << 
		ms->Get_Aggregate_Section("sr_tag", hindex, lindex) << "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sc_req") << " => " << 
		ms->Get_Aggregate_Section("sc_req", hindex, lindex)<< "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sc_ack") << " => " << 
		ms->Get_Aggregate_Section("sc_ack", hindex, lindex)<< "," << endl;
	      ofile << ms->Get_VHDL_Memory_Interface_Port_Name("sc_tag") << " => " << 
		ms->Get_Aggregate_Section("sc_tag", hindex, lindex);
	      comma = ",";
	    }
	}
    }
  return(comma);
}

string vcDataPath::Print_VHDL_IO_Interface_Port_Map(string comma, ostream& ofile)
{
  set<vcPipe*> pipe_set;
  string pipe_id;
  vcModule* parent_module = this->Get_Parent();

  for(int idx = 0; idx < _compatible_inport_groups.size(); idx++)
    {
      vcPipe* p = ((vcInport*) (*(_compatible_inport_groups[idx].begin())))->Get_Pipe();
      pipe_set.insert(p);
    }

  for(set<vcPipe*>::iterator iter = pipe_set.begin();
      iter != pipe_set.end();
      iter++)
    {
      vcPipe* p = (*iter);

      if(p->Get_Parent() == NULL)
	{
	  
	  int hindex, lindex;
	  if(p->Get_Pipe_Module_Section(parent_module,"read", hindex,lindex))
	    {
	      ofile << comma << endl;
	      ofile << p->Get_VHDL_Pipe_Interface_Port_Name("read_req") 
		    << " => " 
		    << p->Get_Pipe_Aggregate_Section("read_req", 
						     hindex, 
						     lindex) 
		    << "," << endl;
	      ofile << p->Get_VHDL_Pipe_Interface_Port_Name("read_ack") 
		    << " => " 
		    << p->Get_Pipe_Aggregate_Section( "read_ack", 
						      hindex, 
						      lindex) 
		    << "," << endl;
	      ofile << p->Get_VHDL_Pipe_Interface_Port_Name("read_data") 
		    << " => " 
		    << p->Get_Pipe_Aggregate_Section("read_data", 
						     hindex, 
						     lindex);
	      comma = ",";
	    }
	}
    }

  pipe_set.clear();
  for(int idx = 0; idx < _compatible_outport_groups.size(); idx++)
    {
      vcPipe* p = ((vcOutport*) (*(_compatible_outport_groups[idx].begin())))->Get_Pipe();
      pipe_set.insert(p);
    }

  for(set<vcPipe*>::iterator iter = pipe_set.begin();
      iter != pipe_set.end();
      iter++)
    {
      vcPipe* p = *iter;

      if(p->Get_Parent() == NULL)
	{
	  int hindex, lindex;
	  if(p->Get_Pipe_Module_Section(parent_module,"write",hindex,lindex))
	    {
	      ofile << comma << endl;
	      ofile << p->Get_VHDL_Pipe_Interface_Port_Name("write_req") 
		    << " => " 
		    << p->Get_Pipe_Aggregate_Section( "write_req", 
						      hindex, 
						      lindex) 
		    << "," << endl;
	      ofile << p->Get_VHDL_Pipe_Interface_Port_Name("write_ack") 
		    << " => " 
		    << p->Get_Pipe_Aggregate_Section("write_ack", 
						     hindex, 
						     lindex) 
		    << "," << endl;
	      ofile << p->Get_VHDL_Pipe_Interface_Port_Name("write_data") 
		    << " => " 
		    << p->Get_Pipe_Aggregate_Section("write_data", 
						     hindex, 
						     lindex);
	      
	      comma = ",";
	    }
	}
    }

  return(comma);
}

string vcDataPath::Print_VHDL_Call_Interface_Port_Map(string comma, ostream& ofile)
{
  set<vcModule*,vcRoot_Compare> called_module_set;
  vcModule* called_module;
  vcModule* parent_module = this->Get_Parent();

  for(int idx = 0; idx < _compatible_call_groups.size(); idx++)
    {
      called_module = ((vcCall*) (*(_compatible_call_groups[idx].begin())))->Get_Called_Module();
      called_module_set.insert(called_module);
    }

  for(set<vcModule*,vcRoot_Compare>::iterator iter = called_module_set.begin();
      iter != called_module_set.end();
      iter++)
    {
      called_module = (*iter);

      int hindex, lindex;
      if(called_module->Get_Caller_Module_Section(parent_module,hindex,lindex))
	{
	  ofile << comma << endl;
	  ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_reqs") << " => " <<
	    called_module->Get_Aggregate_Section("call_reqs", hindex, lindex) << "," << endl;
	  ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_acks") << " => " <<
	    called_module->Get_Aggregate_Section("call_acks", hindex, lindex) << "," << endl;

	  if(called_module->Get_In_Arg_Width() > 0)
	    {
	      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_data") << " => " << 
		called_module->Get_Aggregate_Section("call_data", hindex, lindex) << "," << endl;
	    }

	  ofile << called_module->Get_VHDL_Call_Interface_Port_Name("call_tag") << " => " << 
	    called_module->Get_Aggregate_Section("call_tag", hindex, lindex) << "," << endl;
	  ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_reqs") << " => " << 
	    called_module->Get_Aggregate_Section("return_reqs", hindex, lindex)<< "," << endl;
	  ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_acks") << " => " << 
	    called_module->Get_Aggregate_Section("return_acks", hindex, lindex)<< "," << endl;

	  if(called_module->Get_Out_Arg_Width() > 0)
	    {
	      ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_data") << " => " << 
		called_module->Get_Aggregate_Section("return_data", hindex, lindex)<< "," << endl;
	    }


	  ofile << called_module->Get_VHDL_Call_Interface_Port_Name("return_tag") << " => " << 
	    called_module->Get_Aggregate_Section("return_tag", hindex, lindex);
	  comma = ",";
	}
    }
  return(comma);
}

/////////////////////////////////////////  miscellaneous utility functions ///////////////////////////////////////////

void Generate_Guard_Constants(string& buffering_const, string& guard_flag_const,
				 vector<vcDatapathElement*>& ops, vector<vcWire*>& guard_wires)
{
	assert(guard_wires.size() == ops.size());
	int num_reqs = guard_wires.size();
	int idx;

	// guard flags.
	guard_flag_const = "constant guardFlags : BooleanArray(" + IntToStr(num_reqs-1) + " downto 0) := (";
	buffering_const  = "constant guardBuffering: IntegerArray(" + IntToStr(num_reqs-1) + " downto 0)  := (";
	for(idx = num_reqs-1; idx >= 0; idx--)
	{
		int lidx = (num_reqs - 1) - idx;
		if(guard_wires[idx] != NULL)
		{
			guard_flag_const += IntToStr(lidx) + " => true";
		}
		else
		{
			guard_flag_const += IntToStr(lidx) + " => false";
		}

		int D,S,B;
		vcDatapathElement* dpe = ops[idx];
		if(dpe->Is_Part_Of_Pipelined_Loop(D,S))
		{
			B = dpe->Get_Delay();
		}
		else
		{
			B = 1;
		}
		buffering_const +=  IntToStr(lidx)  + " => " + IntToStr(B);
		if(idx != 0)
		{
			guard_flag_const  += ", ";
			buffering_const   += ", ";
		}
	}
	guard_flag_const += ");";
	buffering_const += ");";
}

void Print_VHDL_Concatenate_Req(string req_id, vector<vcTransition*>& reqs,  ostream& ofile)
{
	
  for(int rI = 0; rI < reqs.size(); rI++)
    {
      ofile << req_id <<  "(" << (reqs.size()-1)-rI << ") <= " 
	    << reqs[rI]->Get_CP_To_DP_Symbol() << ";" << endl;
    }
}


void Print_VHDL_Disconcatenate_Ack(string ack_id, vector<vcTransition*>& acks,  ostream& ofile)
{
  // disconcatenate ackL
  for(int aI = 0; aI < acks.size(); aI++)
    {
      if(acks[aI] != NULL)
	ofile << acks[aI]->Get_DP_To_CP_Symbol() << " <= "
	      << ack_id << "(" << (acks.size()-1) - aI << ");" << endl;
    }
}



void Print_VHDL_Concatenation(string target, vector<vcWire*> wires, ostream& ofile)
{
  ofile << target << " <= ";
  for(int u = 0; u < wires.size(); u++)
    {
      if(u > 0)
	ofile << " & ";
      ofile << wires[u]->Get_VHDL_Signal_Id();
    }
  ofile << ";" << endl;
}



void Print_VHDL_Disconcatenation(string source, int total_width, vector<vcWire*> wires, ostream& ofile)
{
  int lindex = total_width-1;
  for(int u = 0; u < wires.size(); u++)
    {
      ofile << wires[u]->Get_VHDL_Signal_Id() << " <= " << source << "(";
      ofile << lindex << " downto " << (lindex - (wires[u]->Get_Size()-1)) << ");" << endl;
      lindex -= wires[u]->Get_Size();
    }
}

void Print_VHDL_Guard_Concatenation(int num_reqs, string guard_vector, vector<vcWire*>& guard_wires, vector<bool>& guard_complements, ostream& ofile)
{
	int idx;
	for(idx = num_reqs-1; idx >= 0; idx--)
	{
		int lidx = (num_reqs - 1) - idx;
		if(guard_wires[idx] != NULL)
		{
			if(guard_complements[idx])
				ofile << guard_vector << "(" << lidx << ")  <=  not " << guard_wires[idx]->Get_VHDL_Signal_Id() << "(0);" << endl;
			else
				ofile << guard_vector << "(" << lidx << ")  <= " << guard_wires[idx]->Get_VHDL_Signal_Id() << "(0);" << endl;
		}
		else
				ofile << guard_vector << "(" << lidx << ")  <=  '1';" << endl;

	}
}

void Print_VHDL_Guard_Instance(string inst_id, int num_reqs,string guards, string req_unguarded, string ack_unguarded, string req, string ack, bool delay_flag, ostream& ofile)
{

	ofile << inst_id << ": GuardInterface generic map(nreqs => " << num_reqs 
		<< ", delay_flag => " << (delay_flag ? "true" : "false") << ") -- { " << endl
		<< "port map(reqL => " << req_unguarded << "," << endl
		<< "ackL => " << ack_unguarded << "," << endl
		<< "reqR => " << req << "," << endl
		<< "ackR => " << ack << "," << endl
		<< "clk => clk, " << endl
		<< "reset => reset, " << endl
		<< "guards => " << guards << "); -- }" << endl;
}

void Print_VHDL_Guard_Instance(string inst_id, int num_reqs, string buffering, string guard_flags, string guards, 
			string sr_in, string sa_out,  string sr_out, string sa_in,
			string cr_in, string ca_out,  string cr_out, string ca_in, ostream& ofile)
{
	ofile << inst_id << ": SplitGuardInterface generic map(nreqs => " << num_reqs
			<< ", buffering => " << buffering << ", use_guards => " << guard_flags
			<< ") -- {" << endl
			<< "port map(clk => clk, reset => reset," << endl
			<< "sr_in => " << sr_in << "," << endl
			<< "sr_out => " << sr_out << "," << endl
			<< "sa_in => " << sa_in << "," << endl
			<< "sa_out => " << sa_out << "," << endl
			<< "cr_in => " << cr_in << "," << endl
			<< "cr_out => " << cr_out << "," << endl
			<< "ca_in => " << ca_in << "," << endl
			<< "ca_out => " << ca_out << "," << endl
			<< "guards => " << guards << "); -- }" << endl;
}

