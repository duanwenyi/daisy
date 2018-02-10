#include <iostream>
#include <string>
#include <cstdlib>
#include<fstream>
#include <vector>  
using namespace std;
long hex2int(const string& hexStr){
  //char *offset;
  if(hexStr.length() != 8)
  {
    cout << " --< : " << hexStr << endl;
  }
  return strtoul(hexStr.c_str(), NULL, 16);
}

/*int hex2int(const string& hexStr){    int val;
    sscanf(hexStr.c_str(),"%x", & val);
} */
vector< string> split(string str, string pattern){ 
  vector<string> ret;
  if (pattern.empty()) return ret;
  size_t start = 0, index = str.find_first_of(pattern, 0);
  while (index != str.npos) {
    if (start != index)
      ret.push_back(str.substr(start, index - start));
    start = index + 1;
    index = str.find_first_of(pattern, start);
  }
  if (!str.substr(start).empty())
    ret.push_back(str.substr(start));
  return ret;
}

int main(int argc, char **argv){
  // in_file out_file w h last_w
  if(argc != 6 ){
    cout << "parameter: " << argc << endl;
    cout << "Usage: tr in_file out_file w h last_w" << endl;
    return 0;
  }
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
  ifstream in(argv[1]);
  ofstream out(argv[2], std::ofstream::binary);
  int   *rec;
  int    line = 0;
  while (!in.eof()) {
    getline(in, name);
    if (name.empty())  {
      continue;
    }
    sp = split(name, pattern);
    //cout << " # " << sp[0] << endl;
    if(sp.size() != 1){
      cout << "check value @ " << line << " val is : " <<  sp[0] << endl;
    }
    result.push_back(sp[0]);
    line++;
  }
   if(result.size() != nums){
     cout << "Data size not match :" << result.size() << endl;
     return 0;
   }
  rec = new int[nums];
  for(int i=0; i < h; i++ ){
    for(int j=0; j < last_w; j++ ){
            if( ((i*w + j)>= nums)   || ((j*h + i) >= nums) ){
              cout << dec << "overflow : rec @" << (i*w + j)  << " - result@ " << (j*h + i) << " -- ref: " << nums << endl;
              cout << "x : " << j  << " - y:  " << i << endl;
            }
      rec[i*w + j] = hex2int( result.at(j*h + i) );
      //cout << "str : " << result.at(j*h + i) << " - hex : " << hex << rec[i*w + j] << endl;
    }
  }
  if(last_w < w){
    int pos = last_w * h;
    int rem = w - last_w;
        for(int i=0; i < (h-1); i++ ){
          for(int j=0; j < rem; j++ ){
                if( ((i*w + last_w + j)>= nums)   || ((pos + j*(h-1) + i) >= nums) ){
                  cout <<  dec << "overflow :  rec @" << (i*w + last_w + j)  << " - result@ " << (pos + j*h + i) << " -- ref: " << nums << endl;
                }
                rec[i*w + last_w + j] = hex2int( result.at(pos + j*(h-1) + i) );
            //cout << "str : " << result.at(pos + j*(h-1) + i) << " - hex : " << hex << rec[i*w + j] << endl;
          }
        }
    }
    for(int i=0; i < nums; i++){
      printf("%08x ", rec[i]);
      if((i+1)%32 == 0)
          printf("\n");
    }
  
  out.write((char*)rec, nums * sizeof(int) );
  out.close();
  return 0;
}
