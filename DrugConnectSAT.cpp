#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>

using namespace std;

typedef pair<int,int> iPair;

class Utility{
public:
	static string intToString(long int n){
		stringstream ss;
		ss<<n;
		return ss.str();
	}
};

class DrugConnect{
public:
	unsigned int n; //vertices count
	unsigned int m; //edges count
	unsigned int k; //cliques
	long int clauseCount;
	long int varCount;

	iPair* edges;
	bool** adjacency;
	fstream fout;
	char* filePath;

	long int VnCtoVr(int vertex,int component);
	long int EnCtoVr(int edge, int component);
	void addVertexExistenceClauses();
	void addEdgeExistenceClauses();
	void addCliqueExistenceClauses();
	void addVertexPairExistenceClauses();
	void printEqCluases(long int a, long int b, long int c, int type);
	void addMutualExclusionConstraint();
	void addEdgeToVertexClauses();

	DrugConnect(unsigned int n,iPair* edges,unsigned int m,unsigned int k,char* filePath){
		this->n = n;
		this->m = m;
		this->k =k;
		this->clauseCount = 0;
		this->varCount = (n+m)*(long int)k +1;
		this->edges = edges;
		this->filePath = filePath;

		unsigned int i;
		//initialize adjacency matrix
		adjacency = new bool* [this->n];
		for(i=0;i<this->n;i++){
			adjacency[i] = new bool[this->n];
			memset(adjacency[i], false, this->n*sizeof(bool));
		}

		//edges are numbered 0 based
		pair<int,int> edge;
		for(i=0;i<this->m;i++){
			edge = edges[i];
			adjacency[edge.first][edge.second] = true;
			adjacency[edge.second][edge.first] = true;
		}

		fout.open(filePath, ios::out);
		fout<<"p cnf "<<endl;
	}
	~DrugConnect(){
		unsigned int i;
		//Free Adjacency
		for(i=0;i<this->n;i++){
			delete[] adjacency[i];
			adjacency[i] = NULL;
		}
		delete[] adjacency;
		adjacency = NULL;

		delete[] edges;
		edges = NULL;


	    fout.close();
		//including count of clauses/variables in file
		bool flag = true;
	    string line; 

	    fout.open(this->filePath, ios::in);
	    ofstream output("output.temp");

	    while (getline(fout, line)){
	    	if(flag){
	    		output<<"p cnf "<<this->varCount-1<<" "<<this->clauseCount<<endl;
	    		flag = false;
	    		continue;
	    	}
	        output<<line<<endl;
	    }
		
		fout.close();
		output.close();
		//Renaming the file
		remove(this->filePath);
		rename("output.temp",this->filePath);
	}
};

long int DrugConnect::VnCtoVr(int vertex,int component){
	return this->k * (long int)vertex + component + 1;
}

long int DrugConnect::EnCtoVr(int edge, int component){
	long int offset = this->n * (long int)this->k;
	return offset + this->k * (long int)edge + component + 1;
}

void DrugConnect::addVertexExistenceClauses(){
	//max 10 digit variable allowed
	unsigned int vertex,comp;
	for(vertex=0; vertex<this->n;vertex++){
		for(comp=0;comp<this->k;comp++){
			fout<<VnCtoVr(vertex,comp)<<" ";
		}
		fout<<"0"<<endl;
		this->clauseCount++;
	}
}
void DrugConnect::addEdgeExistenceClauses(){
	//max 10 digit variable allowed
	unsigned int edge,comp;
	for(edge=0; edge<this->m;edge++){
		for(comp=0;comp<this->k;comp++){
			fout<<EnCtoVr(edge,comp)<<" ";
		}
		fout<<"0"<<endl;
		this->clauseCount++;
	}
}

void DrugConnect::addCliqueExistenceClauses(){
	unsigned int comp,edge;
	//each clique must have atleast one edge
	for(comp=0;comp<this->k;comp++){
		for(edge=0;edge<this->m;edge++){
			fout<<EnCtoVr(edge,comp)<<" ";
		}
		fout<<"0"<<endl;
		this->clauseCount++;
	}
}

void DrugConnect::addVertexPairExistenceClauses(){
	unsigned int i,j,l,comp,len;
	for(i=0 ;i<this->n; i++){
		for(j=i+1;j<this->n;j++){
			if(adjacency[i][j]==false){
				//~Edge -> ~(V1.V2)
				for(comp=0;comp<this->k;comp++){
					fout<<"-"<<VnCtoVr(i,comp)<<" ";
					fout<<"-"<<VnCtoVr(j,comp)<<" 0"<<endl;
					this->clauseCount++;
				}
			}
		}
	}
}

void DrugConnect::addEdgeToVertexClauses(){
	//E<->V1.V2
	unsigned int comp,edge;
	long int x,y,z;
	for(comp=0;comp<this->k;comp++){
		for(edge=0;edge<this->m;edge++){
			x=EnCtoVr(edge,comp);
			y=VnCtoVr((this->edges)[edge].first,comp);
			z=VnCtoVr((this->edges)[edge].second,comp);

			//write (-x+y).(-x+z).(x+-y+-z)
			printEqCluases(x,y,z,1);
		}
	}
}

void DrugConnect::printEqCluases(long int a, long int b, long int c, int type){

    // type 1 == equivalance of a , b ^ c
    // type 2 == equivalence of a , b => c

    if(type == 1){
        fout << "-" << a << " " << b << " 0" <<endl;
        fout << "-" << a << " " << c << " 0" <<endl;
        fout << "-" << b << " -" << c << " " << a <<" 0" <<endl;
		this->clauseCount += 3;
    }
    else if(type == 2){
        fout << "-" << c << " " << a << " 0" <<endl;
        fout << b << " " << a << " 0" <<endl;
        fout << "-" << a << " -" << b << " " << c <<" 0" <<endl;
		this->clauseCount += 3;
    }
    else
        cout << "type not yet defined!\n";
}

void  DrugConnect::addMutualExclusionConstraint(){

	long int compVariables1[this->n];
	long int compVariables2[this->n];
	unsigned int i,v,j;
	for(i = 0; i < this->k; i++){
		for(v = 0; v < this->n; v++)
			compVariables1[v] = VnCtoVr(v,i);

		for(j = i+1; j < this->k; j++){

			for(int v = 0; v < this->n; v++)
				compVariables2[v] = VnCtoVr(v,j);

			string str = "";
			for(int l = 0; l < n; l++){

				printEqCluases(this->varCount, compVariables1[l], compVariables2[l], 2);
				str.push_back('-');
				str+= Utility::intToString(this->varCount);
				str.push_back(' ');
				this->varCount++;
			}

			fout << str << "0" << endl;
			this->clauseCount++;

			str = "";
			for(int l = 0; l < n; l++){

				printEqCluases(this->varCount, compVariables2[l], compVariables1[l], 2);
				str.push_back('-');
				str += Utility::intToString(this->varCount);
				str.push_back(' ');
				this->varCount++;
			}
			fout << str << "0" << endl;
			this->clauseCount++;

		}
	}
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

	strcpy(inFilePath,argv[1]);
	strcat(inFilePath,".graph");

	strcpy(outFilePath,argv[1]);
	strcat(outFilePath,".satinput");

	ifstream fin(inFilePath);
	if(!fin){
		cout<<"Error opening File. Exiting.";
		return 1;
	}
	int n,m,k,x,y;
	fin>>n>>m>>k;

	//Assuming no repeated edges are there
	iPair* edges = new iPair[m];
	for(int i=0;i<m;i++){
		fin>>x;
		fin>>y;
		edges[i] = make_pair(x-1,y-1);
	}
	//close the file
	fin.close();

	DrugConnect agencies(n,edges,m,k,outFilePath);

	agencies.addVertexExistenceClauses();
	agencies.addEdgeExistenceClauses();
	agencies.addCliqueExistenceClauses();
	agencies.addEdgeToVertexClauses();
	agencies.addVertexPairExistenceClauses();
	agencies.addMutualExclusionConstraint();

	edges= NULL;
	return 0;
}
