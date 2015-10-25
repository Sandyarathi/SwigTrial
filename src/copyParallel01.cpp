/*
 * copyParallel01.cpp
 *
 *  Created on: Oct 24, 2015
 *      Author: sandyarathidas
 */
/*
 * Parallel01.cpp
 *
 *  Created on: Oct 24, 2015
 *      Author: sandyarathidas
 */
/*
 * windroseAlgorithm10.cpp
 *
 *  Created on: Oct 17, 2015
 *      Author: aditi
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <omp.h>
#include <cmath>

using namespace std;

const int NUM_OF_SECTORS = 16;
const int NUM_OF_SPEED = 5;
const int MAX_NUM_DATA_POINTS = pow(2,27);
const int STRIP_WIDTH = 32;
const int NUM_OF_THREADS = 4;

struct MesoData
{
	int maxDataPoints;
	int numDataPoints;
	float* windDir;
	float* windSpd;
};

typedef int outputData[NUM_OF_SECTORS][NUM_OF_SPEED];

typedef int swigData[NUM_OF_SECTORS * NUM_OF_SPEED];

int calcSpeedsBin(float winSpd);

int calcDirectBin(float winDir);

void readData(MesoData & inputData, vector<string> List);

void aggData(MesoData & inputData, outputData & outData);

vector<string> readFileList(string filepath);


int main(){
	struct timeval start, end;
	double delta;

	gettimeofday(&start, NULL);
	cout<<"Hello World... I am processing.." << endl << endl;

	string fileListpath = "../Data/files.txt";
	vector<string> vectorOfFilePaths = readFileList(fileListpath);

	MesoData inputData = {MAX_NUM_DATA_POINTS,
				0,
				(float*)calloc(MAX_NUM_DATA_POINTS, sizeof(float)),
				(float*)calloc(MAX_NUM_DATA_POINTS, sizeof(float))
		};

	outputData outData;

	for(int i=0; i<NUM_OF_SECTORS; i++){
			for(int j=0; j<NUM_OF_SPEED; j++){
				outData[i][j] = 0;
			}
		}

	readData(inputData, vectorOfFilePaths);

	aggData(inputData, outData);

	cout<<"******************** Printing final 2D array *******************************"<< endl;
	for(int i=0; i<NUM_OF_SECTORS; i++){
		for(int j=0; j<NUM_OF_SPEED; j++){
			cout<< outData[i][j] << "\t";
		}
		cout << endl;
	}

	gettimeofday(&end, NULL);
	delta = (end.tv_sec  - start.tv_sec) +
		         ((end.tv_usec - start.tv_usec) / 1.e6);

	//cout<< endl;
	printf("%.6lf seconds elapsed\n", delta);

	free(inputData.windDir);
	free(inputData.windSpd);

}

void callFunction(){

	struct timeval start, end;
		double delta;

		gettimeofday(&start, NULL);
		cout<<"Hello World... I am processing.." << endl << endl;

		string fileListpath = "../Data/files.txt";
		vector<string> vectorOfFilePaths = readFileList(fileListpath);

		MesoData inputData = {MAX_NUM_DATA_POINTS,
					0,
					(float*)calloc(MAX_NUM_DATA_POINTS, sizeof(float)),
					(float*)calloc(MAX_NUM_DATA_POINTS, sizeof(float))
			};

		outputData outData;

		for(int i=0; i<NUM_OF_SECTORS; i++){
				for(int j=0; j<NUM_OF_SPEED; j++){
					outData[i][j] = 0;
				}
			}

		readData(inputData, vectorOfFilePaths);

		aggData(inputData, outData);

		cout<<"******************** Printing final 2D array *******************************"<< endl;
		for(int i=0; i<NUM_OF_SECTORS; i++){
			for(int j=0; j<NUM_OF_SPEED; j++){
				cout<< outData[i][j] << "\t";
			}
			cout << endl;
		}

		gettimeofday(&end, NULL);
		delta = (end.tv_sec  - start.tv_sec) +
			         ((end.tv_usec - start.tv_usec) / 1.e6);

		//cout<< endl;
		printf("%.6lf seconds elapsed\n", delta);

		free(inputData.windDir);
		free(inputData.windSpd);


}

int calcSpeedsBin(float winSpd) {
	if (winSpd == 0)
				return 0;
			else if (winSpd > 0 and winSpd <= 5)
				return 1;
			else if (winSpd > 5 and winSpd <= 15)
				return 2;
			else if (winSpd > 15 and winSpd <= 25)
				return 3;
			else
				return 4;
}

int calcDirectBin(float winDir) {
	// 0-360 - cut into linear line 0-359

	return (int)(winDir / NUM_OF_SECTORS);
}


void readData(MesoData & inputData, vector<string> List) {

	string line, stationId="AR628";
	string path = "../Data/";
	int count = 0;
	cout<<"File list size: "<<List.size()<<endl;
	for (int i = 0; i < List.size(); i++) {
			cout<<"FileName:"<<List[i]<<endl;
			ifstream inputFile(path + List[i]);
			string rowData[6] ;
			string token;
			int j = 0;
			while (getline(inputFile, line)) {
				istringstream lineStream(line);
				j = 0;
				while (getline(lineStream, token, ',')) {
					rowData[j++] = token;
				}

				if(rowData[0] == stationId){
					//cout<<"StationID "<<rowData[0]<<endl;
					inputData.windDir[count] = strtof(rowData[5].c_str(), NULL);
					inputData.windSpd[count] = strtof(rowData[4].c_str(), NULL);
					count++;
				}

				//count++;
				lineStream.clear();
			}
			inputFile.close();
		}

	inputData.numDataPoints = count;
	//cout<<"Number of data points= "<<inputData.numDataPoints<<endl;

}

void aggData(MesoData & inputData, outputData & outData){

	omp_set_num_threads(NUM_OF_THREADS);

	//cout<<"No. of data points = "<< inputData.numDataPoints << endl;

	#pragma omp parallel
	{
		// initialising local output array for each thread
		outputData localOutData;
		for(int i=0; i< NUM_OF_SECTORS; i++){
			for(int j=0; j< NUM_OF_SPEED; j++)
				localOutData[i][j] = 0;
		}

		int maxOfStrip = (inputData.numDataPoints/STRIP_WIDTH)*STRIP_WIDTH;

		#pragma omp for
		for(int k=1; k<= maxOfStrip; k+= STRIP_WIDTH){

			int __attribute__((aligned(64))) D[STRIP_WIDTH] ;
			int __attribute__((aligned(64))) S[STRIP_WIDTH];

			const float* winDirection = &(inputData.windDir[k]);
			const float* winSpeed = &(inputData.windSpd[k]);

			#pragma vector aligned
			for(int c=0; c<STRIP_WIDTH; c++){
				D[c] = calcDirectBin(winDirection[c]);
				S[c] = calcSpeedsBin(winSpeed[c]);
			}

			for(int c=0; c<STRIP_WIDTH; c++){
				if((D[c]<16 && D[c]>=0) && (S[c]<5 && S[c]>=0))
				localOutData[D[c]][S[c]]++;
			}
		}

//		int rem = inputData.numDataPoints % STRIP_WIDTH;
//		int remIndex = rem + (inputData.numDataPoints/STRIP_WIDTH);

		#pragma omp for
		for(int i= maxOfStrip+1; i<= inputData.numDataPoints;i++){
				int d = calcDirectBin(inputData.windDir[i]);
				int s = calcSpeedsBin(inputData.windSpd[i]);

				if((d<16 && d>=0) && (s<5 && s>=0))
				localOutData[d][s]++;
		}

		for(int i=0; i< NUM_OF_SECTORS; i++){
			for(int j=0; j< NUM_OF_SPEED; j++){
				#pragma omp atomic
				outData[i][j] += localOutData[i][j];
			}
		}

	}
}



vector<string> readFileList(string filepath) {
	vector<string> list;
	ifstream inputfile(filepath);
	string line;
	while (getline(inputfile, line)) {
		list.push_back(line);
	}
	inputfile.close();
	return list;
}

///Library/Frameworks/Python.framework/Versions/2.7/Headers/





