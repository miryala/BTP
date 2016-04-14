#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>	
#endif
 
#define MAX_SOURCE_SIZE (0x100000)

void insertFixup(int index,int* LC,int *RC,int *P,int *C);

/*returns a pseudo random number in range [1,r) */
inline long rand_range(long r) {
	int m = RAND_MAX;
	long d, v = 0;
	
	do {
		d = (m > r ? r : m);
		v += 1 + (int)(d * ((double)rand()/((double)(m)+1.0)));
		r -= m;
	} while (r > 0);
	return v;
}

inline int rand_range_re(unsigned int *seed, long r) {
	int m = RAND_MAX;
	long d, v = 0;
	
	do {
		d = (m > r ? r : m);
		v += 1 + (int)(d * ((double)rand_r(seed)/((double)(m)+1.0)));
		r -= m;
	} while (r > 0);
	return v;
}

void leftRotate(int *RC,int *LC,int *P,int x){
	int y;
	y = RC[x];
	RC[x] = LC[y];
	if( LC[y] != 0)
		P[LC[y]] = x;
	P[y] = P[x];
	if(P[x] == 0)
		P[y] = 0;
	else if(x == LC[P[x]])
		LC[P[x]] = y;
	else RC[P[x]] = y;
	LC[y] = x;
	P[x] = y;
}

void rightRotate(int *RC,int *LC,int *P,int y){
	int x;
	x = LC[y];
	LC[y] = RC[x];
	if( RC[x] != 0)
		P[RC[x]] = y;
	P[x] = P[y];
	if(P[y] == 0)
		P[x] = 0;
	else if(y == LC[P[y]])
		LC[P[y]] = x;
	else RC[P[y]] = x;
	RC[x] = y;
	P[y] = x;
}

void RBInsert(long value,int counter,int *node,int *LC,int *RC,int *P,int *C){
	
	int x,y;
	x=1;
	y=0;
	while( node[x] != -1){
		y = x;
		if (value < node[x])
			x = LC[x];
		else
			x = RC[x];
	}
	node[counter] = value;
	P[counter] = y;
	if ( y == 0 )
		P[counter] = 0;
	else if( value < node[y])
		LC[y] = counter;
	else RC[y] = counter;
	insertFixup(counter,LC,RC,P,C);
}

void insertFixup(int index,int* LC,int *RC,int *P,int *C){
	while(C[P[index]]){
		if( P[index] == LC[P[P[index]]]){
			int y = RC[P[P[index]]];
			if(C[y]){
				C[P[index]] = 0;
				C[y] = 0;
				C[P[P[index]]] = 1;
				index = P[P[index]];
			}
			else {
				if(index == RC[P[index]]){
					index = P[index];
					leftRotate(RC,LC,P,index);
				}
				C[P[index]] = 0;
				C[P[P[index]]] = 1;
				rightRotate(RC,LC,P,P[P[index]]);
			}		
		}
		else{
			int y = LC[P[P[index]]];
			if(C[y]){
				C[P[index]] = 0;
				C[y] = 0;
				C[P[P[index]]] = 1;
				index = P[P[index]];
			}
			else {
				if(index == LC[P[index]]){
					index = P[index];
					rightRotate(RC,LC,P,index);
				}
				C[P[index]] = 0;
				C[P[P[index]]] = 1;
				leftRotate(RC,LC,P,P[P[index]]);
			}
		}
	}
	C[1] = 0;
}

int main(int argc, char **argv)
{
	int i,n,j;
	unsigned int seed = 2;
	long r = 10000000;
	i = 400000;
	n = 1;
	j=1;

	while (j < argc){
		char c = argv[j][1];
	printf("%c \n",c);
		switch(c){
			case 'i': i = atoi(argv[j+1]);
				break;
			case 'n': n = atoi(argv[j+1]);
				break;	
			case 'S': seed = atoi(argv[j+1]);
				break;
		}
		j += 2;
	}

	int counter = 1;
	int node[i+n+1];
	int LC[i+n+1];
	int RC[i+n+1];
	int P[i+n+1];
	int C[i+n+1];
	for (j=0; j<i+n+1; j++){
		node[j] = -1;
		LC[j] = 0;
		RC[j] = 0;
		P[j] = -1;
		C[j] = 1;
	}
	C[0] = 0;
	long value = 0;
	clock_t t;
        t = clock();
	/* creating initial tree */
	for (j=0; j<i; j++){
		value = rand_range_re(&seed,r);
		RBInsert(value,counter,node,LC,RC,P,C);
		counter++;
	}
	t = clock() - t;
        double time_taken = ((double)t); // in seconds
	printf(" %f %d micro seconds to execute \n", time_taken,i);
	/*for (j=1;j<i+1;j++){
		printf( "j:%ld node:%ld LC:%ld RC:%ld P:%ld C:%ld\n",j,node[j],LC[j],RC[j],P[j],C[j]);	
	}*/



	return 0;
}
