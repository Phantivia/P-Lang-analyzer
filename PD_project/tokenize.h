#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <regex>
#include <stdlib.h>

using namespace std;
enum tokenType
{
  IDENT = 0,
  KEYWORD = 1,
  OPT_A = 2,
  OPT_B = 3,
  END = 4,
  CONST = 5,
  TYPE_SIGN = 6,
  SYNTAX_VAR = 7,
  SYNTAX_END = 8
};
vector<string> tokenTypeString = {"IDENT", "KEYWORD", "OPT_A", "OPT_B", "END", "CONST", "TYPE_SIGN", "SYNTAX_VAR", "SYNTAX_END"};

class Token
{
public:
  int type;
  int index;
  int line;
  string symbol;
};

class TokenTable
{
public:
  int flag = -1;
  vector<Token> data;
  int indent_count = 0;

  int add_token(int _type, int _index, int _line, string _symbol)
  {
    Token temp_token;
    if (_type == IDENT) //如果添加标识符
    {
      for(int i = 0; i <= data.size(); i++)
      {
        if(i == data.size())
        {
          _index = indent_count;
          indent_count++;
          break; //修复第一个token为标识符时可能出现的越界访问
        }
        if(_symbol == data[i].symbol)
        {
          _index = data[i].index;
          break;
        }
        
      }
    }
    temp_token.index = _index;
    temp_token.type = _type;
    temp_token.line = _line;
    temp_token.symbol = _symbol;
    data.push_back(temp_token);

    return 0;
  }

  int size()
  {
    return data.size();
  }

  void show()
  {
    for (int i = 0; i < data.size(); i++)
    {
      printf("<\" %s \">", data[i].symbol.c_str());
      printf("<Type:%s>", tokenTypeString[data[i].type].c_str());
      printf("<index:%d><in Line:%d>\n", data[i].index, data[i].line);
    }
  }
};

vector<char> ConstTypeSign = {'i', 'l', 'L', 'f', 'F', 'd', 'D'};

vector<string> TypeSign = {"int", "float", "long", "double", "char", "void"};
vector<string> KeyWord = {
    "if", "elif", "else",
    "while", "for", "in",
    "break", "continue",
    "and", "not",
    "import",
    "print",
    "def", "return",
    "global", "local",
    "const"};
vector<string> OptA = {"+", "-", "*", "/", "=", "<", ">"};
vector<string> OptB = {"<=", ">=", "==", "!=", "&&", "||"};
vector<string> End = {
    "(", ")", ",", ";", ".", "[",
    "]", ":", "{", "}", "\"", "\'", "$"};

enum errorType
{
  VAR_EXCEED = 1,
  POINT_ERROR = 2,
  CONEXCEED = 3,
  UNKNOWN_SIGN = 4
};
bool errorFlag = 0;
void error(int Line_idx, string str, int nLine, int errorType, ostream& logout)
{
  cout << " Error at ";
  switch (errorType)
  {
  case VAR_EXCEED:
    printf("<Line: %d Idx:%d>：< %s >变量的长度超过限制!\n", nLine, Line_idx, str.c_str());
    logout << "<Line:" << nLine << " Idx:" << Line_idx << ">：< " << str <<  ">变量的长度超过限制!" << endl;
    errorFlag = 1;
    break;
  case POINT_ERROR:
    printf("<Line: %d Idx:%d>：< %s >常数标识符格式错误!\n", nLine, Line_idx, str.c_str());
    logout << "<Line:" << nLine << " Idx:" << Line_idx << ">：< " << str <<  ">常数标识符格式错误!" << endl;
    errorFlag = 1;
    break;
  case CONEXCEED:
    printf("<Line: %d Idx:%d>：< %s >常量的长度超过限制!\n", nLine, Line_idx, str.c_str());
    logout << "<Line:" << nLine << " Idx:" << Line_idx << ">：< " << str <<  ">常量的长度超过限制!" << endl;
    errorFlag = 1;
    break;
  case UNKNOWN_SIGN:
    printf("<Line: %d Idx:%d>：< %s >未定义字符!\n", nLine, Line_idx, str.c_str());
    logout << "<Line:" << nLine << " Idx:" << Line_idx << ">：< " << str <<  ">未定义字符!" << endl;
    errorFlag = 1;
    break;
  }
}

TokenTable Scannner(string ch)
{
  int idx = 0;
  int Line_idx = 0;
  int nLine = 1;
  TokenTable table;
  while (idx < ch.size())
  {
    while (ch[idx] == ' ' || ch[idx] == 9)
    {
      idx++;
      Line_idx++;
    } //忽略空格和tab
    while (ch[idx] == '\n')
    {
      nLine++;
      idx++;
      Line_idx = 0;
    } //遇到换行符，行数加1
    //处理注释
    if (ch[idx] == '#')
    {
      string temp;
      temp += ch[idx];
      temp += ch[idx+1];
      //行注释
        if (temp != "##")
        {
          while (ch[idx] != '\n')
          {
            idx++;
          }
        }
      //块注释
        else
        {
          int index = ch.find("##", idx+3);
          if(index != string::npos)
          {
            idx = index+3;
            //处理块注释在文本最末端的情况
            if(idx == ch.size())
            {
              continue;
            }
          }
        }
    }
      //处理标识符
      else if (isalpha(ch[idx]) || ch[idx] == '_')
      {
        string word;
        word += ch[idx];
        idx++;
        Line_idx++;
        while (isalpha(ch[idx]) || isdigit(ch[idx]) || ch[idx] == '_')
        {
          word += ch[idx];
          idx++;
          Line_idx++;
        }
        if (word.size() > 20)
        {
          error(Line_idx, word, nLine, 1, logout);
        }
        else
        {
          for (int i = 0; i <= KeyWord.size(); i++)
          {
            if (word == KeyWord[i])
            {
              table.add_token(KEYWORD, i, nLine, word);
              break;
            }
            if (i == KeyWord.size())
            {
              for (int j = 0; j <= TypeSign.size(); j++)
              {
                if (word == TypeSign[j])
                {
                  table.add_token(TYPE_SIGN, j, nLine, word);
                  break;
                }
                if (j == TypeSign.size())
                {
                  table.add_token(IDENT, 0, nLine, word);
                  break;
                }
              }
            }
          }
        }
      } //end of if(isalpha(ch[idx]) || ch[idx] == '_')
      //处理常数
      else if (isdigit(ch[idx]))
      {
        string number;
        int dot_count = 0;
        int x_count = 0;
        int type_flag = 0;
        number += ch[idx];
        idx++;
        Line_idx++;
        while (ch[idx] == 'X' || ch[idx] == 'x' || ch[idx] == '.' || isdigit(ch[idx]))
        {

          if (ch[idx] == 'X' || ch[idx] == 'x')
          {
            x_count++;
          }
          if (ch[idx] == '.')
          {
            dot_count++;
          }

          number += ch[idx];
          idx++;
          Line_idx++;
        }
        if (number.size() > 20)
        {
          error(Line_idx, number, nLine, 3,logout);
        }
        else if (dot_count > 1 || x_count > 1)
        {
          error(Line_idx, number, nLine, 2,logout);
        }
        for (int i = 0; i < ConstTypeSign.size(); i++)
        {
          if (ch[idx] == ConstTypeSign[i])
          {
            type_flag = i;
            idx++;
            Line_idx++;
          }
        }
        table.add_token(CONST, type_flag, nLine, number);
      }
      //处理符号
      else
      {
        string temp;
        temp += ch[idx];
        temp += ch[idx + 1];
        char char1 = temp[0];
        char char2 = temp[1];
        for (int i = 0; i <= OptB.size(); i++)
        {
          if (temp == OptB[i])
          {
            table.add_token(OPT_B, i, nLine, temp);
            idx += 2;
            Line_idx += 2;
            break;
          }

          if (i == OptB.size())
          {
            temp.pop_back();
            for (int j = 0; j <= OptA.size(); j++)
            {
              if (temp == OptA[j])
              {
                table.add_token(OPT_A, j, nLine, temp);
                idx++;
                Line_idx++;
                break;
              }
              if (j == OptA.size())
              {
                for (int k = 0; k <= End.size(); k++)
                {
                  if (temp == End[k])
                  {
                    table.add_token(END, k, nLine, temp);
                    idx++;
                    Line_idx++;
                    break;
                  }
                  if (k == End.size())
                  {
                    error(Line_idx, temp, nLine, 4,logout);
                    idx++;
                    Line_idx++;
                    break;
                  }
                }
              }
            }
          }
        }
      } //end of else
    }   //end of loop
    if(errorFlag == 1)
    {
      table.flag = -1;
    }
    else
    {
      table.flag = 0;
    }
    return table;
  }

string readFileIntoString(string filename)
  {
    ifstream ifile(filename);
    //将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while (buf && ifile.get(ch))
      buf.put(ch);
    //返回与流对象buf关联的字符串
    return buf.str();
  }
