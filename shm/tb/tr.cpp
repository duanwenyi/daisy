#include <iostream>
#include <string>
#include <cstdlib>
#include<fstream>
#include <vector>  
#include <algorithm>
//#include <unistd.h>  

using namespace std;
long hex2int(const string& hexStr)
{
    //char *offset;
    if(hexStr.length() != 8)
        {
            cout << " --< : " << hexStr << endl;
        }
    
    return strtoul(hexStr.c_str(), NULL, 16);
}

string&   replace_all_distinct(string&   str,const   string&   old_value,const   string&   new_value)     
 {     
     for(string::size_type   pos(0);   pos!=string::npos;   pos+=new_value.length())   {     
         if(   (pos=str.find(old_value,pos))!=string::npos   )     
             str.replace(pos,old_value.length(),new_value);     
         else   break;     
     }     
     return   str;     
 }

vector< string> split(string str, string pattern)
{
	vector<string> ret;
	if (pattern.empty()) return ret;
	size_t start = 0, index = str.find_first_of(pattern, 0);
	while (index != str.npos)
	{
		if (start != index)
			ret.push_back(str.substr(start, index - start));
		start = index + 1;
		index = str.find_first_of(pattern, start);
	}
	if (!str.substr(start).empty())
		ret.push_back(str.substr(start));
	return ret;
}

int main(int argc, char **argv)
{
    // in_file out_file w h last_w
    if(argc != 6 ){
        cout << "parameter: " << argc << endl;
        cout << "Usage: tr in_file out_file div_w h last_w" << endl;
        return 0;
    }

    int w = atoi(argv[3]);
    int h = atoi(argv[4]);
    int last_w = atoi(argv[5]);

    int nums;

    cout << " in_file: " << argv[1] << endl;
    cout << "out_file: " << argv[2] << endl;
    cout << "   div_w: " << w << endl;
    cout << "       h: " << h << endl;
    cout << "  last_w: " << last_w << endl;

    if(last_w == 16)
        nums = h * 16;
    else if(last_w < 16)
        nums = (h-1) * 16 + last_w;
    else {
        return 0;
    }

    cout << "Pre-set total nums : " << nums << endl;
    
 	string pattern = " ";
	vector <string> result;
	vector <string> sp;
	string name;
	ifstream in(argv[1]);
	//ofstream out(argv[2], std::ofstream::binary);
    FILE * outfile = fopen(argv[2], "wb");

    int   *rec;
    int    line = 0;

    int    div_num = 16/w;

    int    vect_mode = 0;

    result.clear();
	while (1)
	{
        //cout << " # " << line << endl;
		getline(in, name);
        sp.clear();
        
        //cout << "-line@"<< line << " +items: " << sp.size() << " ::" << name << " ::" << name.length() << endl;
		//if (name.length() == 0)
		if (in.eof())
		{
            cout << "-line@"<< line << " +items: " << sp.size() << " ::" << name << " ::" << name.length() << endl;
            break;
			//continue;
		}

		sp = split(name, pattern);
        
        //cout << " # " << sp[0] << endl;

        //cout << "+line@"<< line << " +items: " << sp.size() << endl;
        if(sp.size() == 16){
            vect_mode = 1;
            //cout << " vect_mode detected " << endl;
        }else if( (sp.size() == w) || (sp.size() == (w*h)) ){
            cout << " full detected " << endl;
        }else if(sp.size() != 1){
            cout << "check value @ " << line << " val is : " <<  sp[0] << " size :" << sp.size() << endl;
        }

        for(int k=0; k < sp.size(); k++){
            string rp = sp.at(k);
            transform(rp.begin(), rp.end(), rp.begin(), ::tolower);
            rp = replace_all_distinct(rp,"s","5");
            rp = replace_all_distinct(rp,"z","2");
            rp = replace_all_distinct(rp,"o","0");
            rp = replace_all_distinct(rp,"i","1");
            rp = replace_all_distinct(rp,"l","1");
            rp = replace_all_distinct(rp,"h","11");	
		
	    rp = replace_all_distinct(rp,"u","11");
	if(!vect_mode){
	    rp = replace_all_distinct(rp,"6","b");	
	    rp = replace_all_distinct(rp,"x","6");            
	}
            result.push_back(rp);
        }

        line++;
	}

    cout << "Read File over !" << endl;
    
    
    if(result.size() != nums){
		cout << "Data size not match :" << result.size() << endl;
        return 0;
    }else{
		cout << "Data size matched :" << result.size() << endl;
    }

    rec = new int[nums];

    if(vect_mode){
        cout << " vect_mode detected " << endl;
        for(int i=0; i < nums; i++ ){
            rec[i] = hex2int( result.at(i) );
        }

    }else if(last_w != 16){
        for(int k=0; k < last_w; k++ ){
            rec[k] = hex2int( result.at(k) );
        }
    }else if(w==3){

        for(int k=0; k < div_num; k++ ){
            for(int i=0; i < h; i++ ){
                for(int j=0; j < w; j++ ){
                    
                    if( ((i*16 + k*w + j)>= nums)   || ((j + i*w + k*w*h) >= nums) ){
                        cout << dec << "overflow : rec @" << (i*16 + k*w + j)  << " - result@ " << (j + i*w + k*w*h) << " -- ref: " << nums << endl;
                        cout << "x : " << j  << " - y:  " << i << " -k: " << k << endl;
                    }
                    
                    rec[i*16 + k*w + j] = hex2int( result.at(j + i*w + k*w*h) );
                }
            }
        }

        // last line
        for(int k=0; k < h; k++ ){
            rec[(k+1)*16-1] = hex2int( result.at(nums - 50 + k) );
            
        }
        
    }else{

        
        for(int k=0; k < div_num; k++ ){
            for(int i=0; i < h; i++ ){
                for(int j=0; j < w; j++ ){
                    
                    if( ((i*16 + k*w + j)>= nums)   || ((j + i*w + k*w*h) >= nums) ){
                        cout << dec << "overflow : rec @" << (i*16 + k*w + j)  << " - result@ " << (j + i*w + k*w*h) << " -- ref: " << nums << endl;
                        cout << "x : " << j  << " - y:  " << i << " -k: " << k << endl;
                    }
                    
                    rec[i*16 + k*w + j] = hex2int( result.at(j + i*w + k*w*h) );
                }
            }
        }

    }

    for(int i=0; i < nums; i++){
        printf("%08x ", rec[i]);

        if((i+1)%16 == 0)
            printf("\n");
    }

    fwrite((char*)rec, 1, nums * 4, outfile);
    fclose(outfile);
    
    return 0;
}
