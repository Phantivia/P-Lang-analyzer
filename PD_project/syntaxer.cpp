
#include <fstream>
#include <ostream>
#include <stack>
#include <queue>
using namespace std;

ofstream logout;
#include "tokenize.h"
#include "macro.h"


class Summoner
{
public:
    Token from;
    vector<Token> to;
    vector<string> first;

    void show()
    {
        logout << from.symbol << "-->";
        for (int i = 0; i < to.size(); i++)
        {
            if (i != 0)
            {
                logout << " + ";
            }
            logout << to[i].symbol;
        }
        logout << endl;
    }
};

vector<Summoner> summoners;

void fill_map(map<string, vector<Summoner>> &group, vector<Summoner> slist)
{
    vector<Summoner> space = {};
    for (int i = 0; i < slist.size(); i++)
    {
        if (group.find(slist[i].from.symbol) == group.end())
        {
            group[slist[i].from.symbol] = space;
        }
        group[slist[i].from.symbol].push_back(slist[i]);
    }
}

void unique_push(vector<string> &vec, string &elem)
{
    if (elem == "&")
    {
        logout << "UNEXPECTED & sign";
    }
    if (find(vec.begin(), vec.end(), elem) == vec.end())
    {
        vec.push_back(elem);
    }
}

vector<string> cal_first(map<string, vector<Summoner>> &group, string from)
{
    vector<string> first;
    string end = "$";

    //直接查找终结符的first就返回终结符
    if (group.find(from) == group.end())
    {
        unique_push(first, from);
        return first;
    }

    for (int i = 0; i < group[from].size(); i++)
    {

        Summoner &sum = group[from][i];
        if (sum.to[0].type == SYNTAX_END)
        {
            unique_push(first, sum.to[0].symbol);
            continue;
        }
        else
        {
            vector<string> temp;
            temp = cal_first(group, sum.to[0].symbol);
            for (int i = 0; i < temp.size(); i++)
            {
                if (temp[i] != "$")
                {
                    unique_push(first, temp[i]);
                }
            }
            for (int i = 0; i <= sum.to.size(); i++)
            {
                if (sum.to[i].symbol == from) //跳过同一符号
                {
                    continue;
                }
                if (i == sum.to.size())
                {
                    unique_push(first, end);
                    break;
                }
                if (sum.to[i].type == SYNTAX_VAR)
                {
                    temp = cal_first(group, sum.to[i].symbol);
                    for (int i = 0; i < temp.size(); i++)
                    {
                        unique_push(first, temp[i]);
                    }
                    if (find(temp.begin(), temp.end(), "$") != temp.end())
                    {
                        continue;
                    }
                    else
                    {

                        break;
                    }
                }
                else
                {
                    temp = cal_first(group, sum.to[i].symbol);
                    for (int i = 0; i < temp.size(); i++)
                    {
                        unique_push(first, temp[i]);
                    }
                    break;
                }
            }
        }
    }
    return first;
}
//在产生式s的to列表中查找symbol为symbol的token，找到返回index，找不到返回-1
int search_symbol(Summoner s, string symbol)
{
    vector<Token>::iterator iter = s.to.begin();
    int index = 0;
    for (; iter != s.to.end(); iter++, index++)
    {
        if (iter->symbol == symbol)
        {
            return index;
        }
    }
    return -1;
}

vector<string> cal_follow(map<string, vector<Summoner>> &group, vector<Summoner> &sums, string from)
{
    vector<string> follow;
    vector<string> temp;
    string end = "$";
    int index = 0;
    bool has_end = false;
    for (int i = 0; i < sums.size(); i++)
    {
        Summoner &s = sums[i];
        index = search_symbol(s, from);
        //若B->$是一个产生式...
        if (s.from.symbol == from && s.to[0].symbol == "$")
        {
            unique_push(follow, end);
        }
        //若A→aBp是一个产生式...
        else if (s.from.symbol != from && index != -1)
        {
            if (index != (s.to.size() - 1))
            {
                temp = cal_first(group, s.to[index + 1].symbol);
                for (int i = 0; i < temp.size(); i++)
                {
                    if (temp[i] == end)
                    {
                        has_end = true;
                        continue;
                    }
                    if (temp[i] != end)
                    {
                        unique_push(follow, temp[i]);
                    }
                }
            }
            if (has_end || index == (s.to.size() - 1))
            {
                temp = cal_follow(group, sums, s.from.symbol);
                {
                    for (int i = 0; i < temp.size(); i++)
                    {
                        unique_push(follow, temp[i]);
                    }
                }
            }
        }
    } //end of loop
    return follow;
}

bool token_match(Token token, string symbol)
{
    if (token.type == IDENT && symbol == "id")
    {
        return true;
    }
    else if (token.symbol == symbol)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//统计AST节点数量的全局变量
int tree_node_count = 0;

class ASTNode
{
public:
    int id;
    int access_index;
    int depth = 0;
    int layer_index = 0;
    float y = 0;
    float x = 0;

    string symbol = "";
    vector<ASTNode> children;
    ASTNode *parent = NULL;

    ASTNode()
    {
        id = tree_node_count;
        tree_node_count++;
        access_index = 0;
    }

    ASTNode *get_next()
    {
        if (access_index < children.size())
        {
            access_index++;
            return &(children[access_index - 1]);
        }
        else if (access_index == children.size())
        {
            return parent->get_next();
        }
        else
        {
            logout << "UNEXPECTED TREE ACCESS ERROR" << endl;
            return NULL;
        }
    }
};

int LL_step(
    stack<Token> &syntax_stack, queue<Token> &token_queue,
    map<string, vector<Summoner>> summoner_groups,
    map<string, vector<string>> first_map,
    map<string, vector<string>> follow_map,
    ASTNode *&p)
{
    Token syntax = syntax_stack.top();
    Token code = token_queue.front();

    if (code.type == IDENT)
        code.symbol = "id";
    else if (code.type == CONST)
        code.symbol = "const";

    Summoner use_summoner;
    string end_symbol = "$";
    int flag = 3;

    //语法栈取出一个终结符的情况
    if (syntax.type == SYNTAX_END)
    {
        flag = 2;
    }

    //取出一个变量的情况
    else
    {
        vector<string> &symbol_first = first_map[syntax.symbol];
        if (find(symbol_first.begin(), symbol_first.end(), code.symbol) != symbol_first.end())
        {
            vector<Summoner> &sumlist = summoner_groups[syntax.symbol];
            for (int i = 0; i <= sumlist.size(); i++)
            {
                if (i == sumlist.size())
                {
                    logout << endl
                         << "UNEXPECTED ERROR" << endl;
                    flag = 3;
                    break;
                }
                if (find(sumlist[i].first.begin(), sumlist[i].first.end(), code.symbol) != sumlist[i].first.end())
                {
                    use_summoner = sumlist[i];
                    flag = 0;
                    break;
                }
            }
        }
        else
        {
            if (find(symbol_first.begin(), symbol_first.end(), end_symbol) != symbol_first.end())
            {
                vector<string> &symbol_follow = follow_map[syntax.symbol];
                if (find(symbol_follow.begin(), symbol_follow.end(), code.symbol) != symbol_follow.end())
                {
                    flag = 1;
                }
                else
                {
                    logout << endl
                         << "SYNTAX ERROR" << endl;
                    flag = 3;
                }
            }
        }
    }
    if (flag == 0)
    {
        syntax_stack.pop();

        for (int i = use_summoner.to.size() - 1; i >= 0; i--)
        {
            syntax_stack.push(use_summoner.to[i]);
        }

        for (int i = 0; i < use_summoner.to.size(); i++)
        {
            ASTNode temp;
            temp.symbol = use_summoner.to[i].symbol;
            temp.parent = p;
            temp.depth = (temp.parent->depth) + 1;
            p->children.push_back(temp);
        }

        logout << "Use summoner :";
        use_summoner.show();
        logout << endl;
    }
    else if (flag == 1)
    {
        syntax_stack.pop();
        logout << "Use summoner :";
        logout << end_symbol << endl;
    }
    else if (flag == 2)
    {
        logout << "Use summoner :";
        logout << syntax.symbol << endl;
        syntax_stack.pop();
        token_queue.pop();
    }
    else if (flag == 3)
    {
        logout << endl
             << "SYNTAX ANALYZE ERROR " << endl;
    }

    //获得next位置的AST节点指针
    p = p->get_next();
    return flag;
}

void show_stack(stack<Token> s)
{
    vector<Token> temp;
    while (s.size() > 0)
    {
        temp.push_back(s.top());
        s.pop();
    }
    for (int i = temp.size() - 1; i >= 0; i--)
    {
        logout << temp[i].symbol << " ";
    }
}

void show_queue(queue<Token> q)
{
    while (q.size() > 0)
    {
        string temp = q.front().symbol;
        logout << "<" << temp << ">";
        q.pop();
    }
}

void assign_layer_index(ASTNode *head)
{
    queue<ASTNode *> q;
    q.push(head);
    int current_depth = 0;
    int current_index = 0;
    while (q.size() != 0)
    {
        ASTNode *cur = q.front();

        for (int i = 0; i < cur->children.size(); i++)
        {
            q.push(&(cur->children[i]));
        }

        if (current_depth < cur->depth)
        {
            current_depth = cur->depth;
            current_index = 0;
        }

        cur->layer_index = current_index;
        current_index++;
        q.pop();
    }
}

vector<ASTNode *> get_pins(ASTNode *head)
{
    vector<ASTNode *> pins;
    queue<ASTNode *> q;
    q.push(head);

    while (q.size() != 0)
    {
        ASTNode *cur = q.front();
        pins.push_back(cur);

        for (int i = 0; i < cur->children.size(); i++)
        {
            q.push(&(cur->children[i]));
        }
        q.pop();
    }
    return pins;
}

int main()
{
    logout.open("log.txt", ios::out | ios::trunc);

    KeyWord.push_back("id");

    char *filename = "syntax.txt";
    ifstream file;
    file.open(filename, ios::in);
    if (!file.is_open())
    {
        logout << "语法文件缺失！" << endl;
        logout << "Exited with exit code 1" << endl;
        return 1;
    }
    string strLine;
    TokenTable table;

    //逐行读入生成产生式
    while (getline(file, strLine))
    {
        if (strLine.empty())
        {
            continue;
        }
        table = Scannner(strLine);

        if(table.flag == -1)
        {
            logout << "Syntax Tokenize error occured" << endl;
            logout << "Exited with exit code 1" << endl;
            logout.close();
            return 1;
        }

        for (int i = 0; i < table.size(); i++)
        {
            if (table.data[i].symbol == "\'" && i > 0)
            {
                table.data[i - 1].symbol += "\'";
                table.data.erase(table.data.begin() + i);
            }
        }
        for (int i = 0; i < table.size(); i++)
        {
            table.data[i].type = (table.data[i].type == IDENT) ? SYNTAX_VAR : SYNTAX_END;
        }
        table.show();
        Summoner temp;
        temp.from = table.data[0];
        table.data.erase(table.data.begin());
        table.data.erase(table.data.begin());
        temp.to = table.data;
        summoners.push_back(temp);
        printf("-------------------------\n");
    }

    // for (int i = 0; i < summoners.size(); i++)
    // {
    //     summoners[i].show();
    // }

    //生成容器
    map<string, vector<Summoner>> summoner_groups;
    map<string, vector<string>> first_map;
    map<string, vector<string>> follow_map;

    //给产生式分组
    fill_map(summoner_groups, summoners);

    //填充FIRST容器
    map<string, vector<Summoner>>::iterator iter = summoner_groups.begin();
    for (; iter != summoner_groups.end(); iter++)
    {
        first_map[iter->first] = cal_first(summoner_groups, iter->first);
    }

    //填充FOLLOW容器
    iter = summoner_groups.begin();
    for (; iter != summoner_groups.end(); iter++)
    {
        follow_map[iter->first] = cal_follow(summoner_groups, summoners, iter->first);
    }

    //填充每个产生式的first
    iter = summoner_groups.begin();
    for (; iter != summoner_groups.end(); iter++)
    {
        vector<Summoner> &sumlist = iter->second;
        for (int i = 0; i < sumlist.size(); i++)
        {
            sumlist[i].first = cal_first(summoner_groups, sumlist[i].to[0].symbol);
        }
    }
    //读入代码文件
    string code_string = readFileIntoString("input.txt");
    code_string = macro_process(code_string);
    TokenTable code_table = Scannner(code_string);
    if(code_table.flag == -1)
        {
            logout << "Code Tokenizing error occured" << endl;
            logout << "Exited with exit code 1" << endl;
            logout.close();
            return 1;
        }
    queue<Token> code_token_queue;
    for (int i = 0; i < code_table.size(); i++)
    {
        code_token_queue.push(code_table.data[i]);
    }

    stack<Token> syntax_token_stack;

    //初始化符号栈
    Token end_token;
    end_token.index = 0, end_token.line = 0;
    end_token.symbol = "$", end_token.type = SYNTAX_END;
    Token start_token;
    start_token.index = 0, start_token.line = 0;
    start_token.symbol = "S\'", start_token.type = SYNTAX_VAR;
    syntax_token_stack.push(end_token);
    syntax_token_stack.push(start_token);

    //初始化AST根节点
    ASTNode root;
    root.symbol = "S\'";
    ASTNode *p = &root;

    int flag = 0;
    int step = 0;
    while (code_token_queue.size() != 0)
    {
        step++;
        logout << "Step. " << step << ": "
             << "SYNTAX: ";
        show_stack(syntax_token_stack);
        logout << "  CODE:  ";
        show_queue(code_token_queue);
        // p as reference
        flag = LL_step(syntax_token_stack, code_token_queue, summoner_groups, first_map, follow_map, p);
        if(flag == 3){
            logout << "Exited with exit code 0"  << endl;
            return 1; 
        }
    }

    ASTNode *q = &root;
    assign_layer_index(q);
    vector<ASTNode *> node_list = get_pins(&root);

    float max_depth = 0;
    float max_width = 0;
    vector<float> widths;
    for (int i = 0; i < node_list.size(); i++)
    {
        ASTNode &n = *(node_list[i]);
        if (n.depth > max_depth)
        {
            widths.push_back(max_width);
            max_width = 0;
            max_depth = n.depth;
        }
        else
        {
            max_width += 1;
        }
    }

    widths.push_back(max_width);

    for (int i = 0; i < node_list.size(); i++)
    {
        ASTNode &n = *(node_list[i]);
        n.y = n.depth/(max_depth + 1);
        n.x = (n.layer_index + 1)/(widths[n.depth] + 2);
    }

    //写入文件
    ofstream ofile;
    ofile.open("output.txt", ios::out | ios::trunc);

    //id=0 symbol=$ depth=1 index=0 parent_id=0
    for (int i = 0; i < node_list.size(); i++)
    {
        ASTNode &n = *(node_list[i]);
        ofile << "id=" << n.id << " ";
        ofile << "symbol=" << n.symbol << " ";
        ofile << "y=" << n.y << " ";
        ofile << "x=" << n.x << " ";
        if (n.parent == NULL)
        {
            ofile << "parent_id="
                  << "0" << endl;
        }
        else
        {
            ofile << "parent_id=" << n.parent->id << endl;
        }
    }
    ofile.close();
    logout.close();
    logout << "Finished with exit code 0"  << endl;
    return 0;
}