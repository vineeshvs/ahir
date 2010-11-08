using namespace std;
#include <AaIncludes.h>
#include <AaEnums.h>
#include <AaUtil.h>

string Tab_(unsigned int n)
{
  string ret_string = "";
  for(unsigned int i=0; i < n; i++)
    ret_string += '\t';
  return(ret_string);
}

string IntToStr(unsigned int x)
{
  ostringstream string_stream(ostringstream::out);
  string_stream << x;
  return(string_stream.str());
}

bool StringCompare::operator() (string s11, string s21) const
{
  const char *s1 = s11.c_str ();
  const char *s2 = s21.c_str ();
  for (int i = 0; s2[i]; i++)
    {
      if (!s1[i])
	return ((!s2[i]) ? false : true);
      else if (s1[i] < s2[i])
	return true;
      else if (s1[i] > s2[i])
	return false;
    }
  return false;
};

bool Is_Shift_Operation(AaOperation op)
{
  return(op == __SHL || op == __SHR);
}
bool Is_Compare_Operation(AaOperation op)
{
  return(op == __LESS || op == __GREATER || op == __LESSEQUAL || op == __GREATEREQUAL 
	 || op == __EQUAL || op == __NOTEQUAL);
}

string C_Name(AaOperation op)
{
  string ret_string = "__undefined";
  switch(op)
    {
    case __OR:
      ret_string = "__OR";
      break;
    case __AND:
      ret_string = "__AND";
      break;
    case __NOR:
      ret_string = "__NOR";
      break;
    case __NAND:
      ret_string = "__NAND";
      break;
    case __XOR:
      ret_string = "__XOR";
      break;
    case __XNOR:
      ret_string = "__XNOR";
      break;
    case __SHL:
      ret_string = "__SHL";
      break;
    case __SHR:
      ret_string = "__SHR";
      break;
    case __PLUS:
      ret_string = "__PLUS";
      break;
    case __MINUS:
      ret_string = "__MINUS";
      break;
    case __DIV:
      ret_string = "__DIV";
      break;
    case __MUL:
      ret_string = "__MUL";
      break;
    case __EQUAL:
      ret_string = "__EQUAL";
      break;
    case __NOTEQUAL:
      ret_string = "__NOTEQUAL";
      break;
    case __LESS:
      ret_string = "__LESS";
      break;
    case __LESSEQUAL:
      ret_string = "__LESSEQUAL";
      break;
    case __GREATER:
      ret_string = "__GREATER";
      break;
    case __GREATEREQUAL:
      ret_string = "__GREATEREQUAL";
      break;
    case __NOT:
      ret_string = "__NOT";
      break;
    default:
      cerr << "Error: unrecognized operation" << endl;
    }
  return(ret_string);
}

string Aa_Name(AaOperation op)
{
  string ret_string = "__undefined";
  switch(op)
    {
    case __OR:
      ret_string = "|";
      break;
    case __AND:
      ret_string = "&";
      break;
    case __NOR:
      ret_string = "~|";
      break;
    case __NAND:
      ret_string = "~&";
      break;
    case __XOR:
      ret_string = "^";
      break;
    case __XNOR:
      ret_string = "~^";
      break;
    case __SHL:
      ret_string = "<<";
      break;
    case __SHR:
      ret_string = ">>";
      break;
    case __PLUS:
      ret_string = "+";
      break;
    case __MINUS:
      ret_string = "-";
      break;
    case __DIV:
      ret_string = "/";
      break;
    case __MUL:
      ret_string = "*";
      break;
    case __EQUAL:
      ret_string = "==";
      break;
    case __NOTEQUAL:
      ret_string = "!=";
      break;
    case __LESS:
      ret_string = "<";
      break;
    case __LESSEQUAL:
      ret_string = "<=";
      break;
    case __GREATER:
      ret_string = ">";
      break;
    case __GREATEREQUAL:
      ret_string = ">=";
      break;
    case __NOT:
      ret_string = "~";
      break;
    default:
      cerr << "Error: unrecognized operation" << endl;
    }
  return(ret_string);
}
