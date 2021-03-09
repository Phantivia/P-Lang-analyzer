#include <iostream>
#include <string>
#include <regex>
#include <map>
using namespace std;
string string_rep(string input, string from, string to)
{
    int find_pos = 0;
    int where_pos = 0;
    while(where_pos = input.find(from, find_pos), where_pos != string::npos)
    {
        input.replace(where_pos, from.size(), to);
        find_pos = where_pos + to.size();
    }
    return input;
}
void show_mat(smatch mat)
{
    for(int i = 0; i<mat.size(); i++)
    {
        cout<<"No."<<i<<"   ";
        cout<<mat.str(i)<<endl;
    }
}
string macro_process(string ch)
{
    string _r = "#define\\s+"\
    "((\\w+)|((\\w+)\\(((\\w+,)*)(\\w+)\\)))\\s+"\
    "(.+)";
    regex reg(_r);
    smatch mat;
    while(regex_search(ch, mat, reg))
    {
        if(mat.str(2).size() != 0)
        {
            ch = string_rep(ch, mat.str(2), mat.str(7));
        }
        else
        {   
            map<string,string> var_map;
            string var_arr(mat.str(5));
            int comma_index = 0;
            int find_index = 0;
            int var_count = 0;
            
            while(comma_index = var_arr.find(',',find_index), comma_index != string::npos)
            {   
                //可能有点问题
                string clip = var_arr.substr(find_index, comma_index - find_index);
                var_map[clip] = "";
                find_index = comma_index+1;
                if(find_index == var_arr.size())
                break;
            }
            var_map[mat.str(7)] = "";

            var_count = var_map.size();
            string var_r;
            for(int i = 0; i<var_count-1; i++)
            {
                var_r += "(\\w+),";
            }
            var_r = mat.str(4) + "\\(" + var_r + "(\\w+)\\)";
            regex capture(var_r);

            smatch s_mat;
            string::const_iterator begin, end;
            begin = ch.begin() + mat.position() + mat.length();
            end = ch.end();
            while(regex_search(begin, end, s_mat, capture))
            {
                string insert(mat.str(8));
                map<string,string>::iterator iter = var_map.begin();
                for( int i = 1; iter != var_map.end(); iter++,i++)
                {
                    iter->second = s_mat.str(i);
                    insert = string_rep(insert, iter->first, iter->second);
                }
                ch = string_rep(ch, s_mat.str(0), insert);
                begin += insert.size();
            }
        }
    ch.erase(mat.position(),mat.position()+mat.length());
    }//end of while
    
    return ch;
}