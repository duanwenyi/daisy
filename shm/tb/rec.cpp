#include <iostream>
#include <string>
#include <cstdlib>
#include<fstream>
#include <vector>  
#include <algorithm>
using namespace std;
int hex2int(const string& hexStr){    
  //char *offset;    
  if(hexStr.length() != 8)        {            
    cout << " --< : " << hexStr << endl;        
  }        
  return strtoul(hexStr.c_str(), NULL, 16);
}

vector< string> split(string str, string pattern){ 
  vector<string> ret;
  if (pattern.empty()) 
    return ret; 
  size_t start = 0, index = str.find_first_of(pattern, 0); 
  while (index != str.npos) {  
    if (start != index)   
      ret.push_back(str.substr(start, index - start));  
    start = index + 1;  
    index = str.find_first_of(pattern, start); 
  } if (!str.substr(start).empty())  
    ret.push_back(str.substr(start)); 
  return ret;
}

int main(int argc, char **argv){    // in_file out_file w h last_w    if(argc != 6 ){        cout << "parameter: " << argc << endl;        cout << "Usage: tr in_file out_file w h last_w" << endl;        return 0;    }
    int w = atoi(argv[3]);    
    int h = atoi(argv[4]);    
    int last_w = atoi(argv[5]);
    int nums = w * (h-1) + last_w;
  
    cout << " in_file: " << argv[1] << endl;
    cout << "out_file: " << argv[2] << endl;
    cout << "       w: " << w << endl;
    cout << "       h: " << h << endl;
    cout << "  last_w: " << last_w << endl;
    cout << "Pre-set total nums : " << nums << endl;
    string pattern = " ";
    vector <string> result;
    vector <string> sp;
    string name;
    ifstream in(argv[1]); //ofstream out(argv[2], std::ofstream::binary);
    FILE * outfile = fopen(argv[2], "wb");
    int   *rec;
    int    line = 0;
    string rp;
    int    vect_mode = 0;
    while (!in.eof()) {  
      getline(in, name);
      sp.clear();
      //cout << "-line@"<< line << " +items: " << sp.size() << " ::" << name << " ::" << name.length() << endl;
      if (name.length() == 0)  {
        cout << "-line@"<< line << " +items: " << sp.size() << " ::" << name << " ::" << name.length() << endl;
        continue;
      }
     sp = split(name, pattern);
     for(int k=0; k < sp.size(); k++){
       rp = sp.at(k);
       transform(rp.begin(), rp.end(), rp.begin(), ::tolower);
       result.push_back(rp);
     }
     line++;
     if(sp.size() != w){
       cout << " last line detected @" << line << " +size "<< sp.size() << endl;
       //break;
     }
    }
    cout << " ref sizeof(int) :" << sizeof(int) << endl;
    cout << " ref sizeof(long):" << sizeof(long) << endl;
    cout << " Begin Recovery items :" << result.size() << endl;
    if(result.size() != nums){
      cout << "Data size not match :" << result.size() << endl;
      return 0;
    }
    rec = new int[nums];
   for(int i=0; i < nums; i++ ){
     rec[i] = hex2int( result.at(i) );
   }
    /*    for(int i=0; i < nums; i++){        printf("%08x ", rec[i]);
        if((i+1)%16 == 0)            printf("\n");    }    */        //out.write((char*)rec, sizeof(rec) );    //out.close();
    fwrite((char*)rec, 1, nums * 4, outfile);
    fclose(outfile);
    cout << "Rec Done !"  << endl;
    return 0;
}
