
#include<iostream>
#include<fstream>
#include<cstring>
#include<sstream>
#include<string>
using namespace std;

string intToString(int n){
	stringstream ss;
	ss<<n;
	return ss.str();
}

int main(int argc, char* argv[]){

	//Getting file names
	if(!argv[1]){
		cout<<"No file Given.";
		return -1;
	}

	int len = strlen(argv[1]);

	char inFilePath[len+15];
	char outFilePath[len+15];
	char graphFile[len+15];

	strcpy(inFilePath,argv[1]);
	strcat(inFilePath,".satoutput");

	strcpy(outFilePath,argv[1]);
	strcat(outFilePath,".subgraphs");

	strcpy(graphFile,argv[1]);
	strcat(graphFile,".graph");

	//cout<<inFilePath<<" "<<outFilePath<<" "<<graphFile<<endl;

	ifstream in;
	in.open(inFilePath);
	
	ofstream out;
	out.open(outFilePath);
	
	string s;

	in >> s;
	
	long int var;
	int n, m, k;
	ifstream graph(graphFile);
	
	graph >> n >> m >> k;
	bool truth[n][k];
	if( s.compare("SAT") == 0){
		//Assuming SAT will give value assignment for all the variables.
		in >> var;
		//cout << "Value of Var = " << var << endl;
		for(int i =0; i < n; i++){
			for(int j =0 ; j< k; j++){
				if(var > 0)
					truth[i][j] = true;
				else if(var < 0)
					truth[i][j] = false;
				else break; // should only never happen
				in >> var;
				//cout << "Value of Var = " << var << endl;
			}
		}
		int count;
		string str;
		for(int i = 0; i < k; i++){
			count = 0;
			str = "";
			for(int j = 0; j < n; j++){
				if(truth[j][i]){
					count++;
					string s;
					str += intToString(j+1) + " ";
				}
			}
			if(count > 0){
				out << "#" << i+1 << " " << count << endl;
				str.erase(str.length()-1,1);
				out << str << endl;
			}
		}
	}
	else out << 0;

	out.close();
	graph.close();
	in.close();
	return 0;
}

