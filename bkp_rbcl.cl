#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
//#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics: enable

void leftRotate(__global int *RC,__global int *LC,__global int *P,int x,__global int *R){
	__private int y;
	y = RC[x];
	RC[x] = LC[y];
	if( LC[y] != 0)
		P[LC[y]] = x;
	P[y] = P[x];
	if(P[x] == 0){
		P[y] = 0;
		R[0] = y;
		}
	else if(x == LC[P[x]])
		LC[P[x]] = y;
	else RC[P[x]] = y;
	LC[y] = x;
	P[x] = y;
}

void rightRotate(__global int *RC,__global int *LC,__global int *P,int y,__global int *R){
	__private int x;
	x = LC[y];
	LC[y] = RC[x];
	if( RC[x] != 0)
		P[RC[x]] = y;
	P[x] = P[y];
	if(P[y] == 0){
		P[x] = 0;
		R[0] = x;
		}
	else if(y == LC[P[y]])
		LC[P[y]] = x;
	else RC[P[y]] = x;
	RC[x] = y;
	P[y] = x;
}


int RBInsert(long value,int counter,__global int *node,__global int *LC,__global int *RC,__global int *P,__global int *C,__global int *L,__global int *R){
	
	__private int x,y;
	x=1;
	y=0;
	while( node[x] != -1){
		y = x;
		if (value < node[x])
			x = LC[x];
		else
			x = RC[x];
	}
	if( L[y] != 1){
		L[counter] = 1;
		node[counter] = value;
		P[counter] = y;
		if ( y == 0 ){
			P[counter] = 0;
			R[0] = counter;
			}
		else if( value < node[y])
			LC[y] = counter;
		else RC[y] = counter;
		//insertFixup(/*counter/*,LC,RC,P,C,L,R*/);
		/**************************** Insert Fixup ******************************************/


		while(C[P[counter]]){
		if( (P[counter] == LC[P[P[counter]]]) && L[counter]==0 && L[P[counter]]==0 && L[P[P[counter]]]==0 && L[RC[P[P[counter]]]]==0 ){
			__private int l1,l2,l3,l4;
			l1 = atomic_cmpxchg(&L[counter],0,1);
			l2 = atomic_cmpxchg(&L[P[counter]],0,1);
			l3 = atomic_cmpxchg(&L[P[P[counter]]],0,1);
			l4 = atomic_cmpxchg(&L[RC[P[P[counter]]]],0,1);
			//L[counter]=1; L[P[counter]]=1;  L[P[P[counter]]]=1; L[RC[P[P[counter]]]]=1;
			if(!l1 && !l2 && !l3 && !l4){
				__private int z = RC[P[P[counter]]];
				if(C[z]){
					C[P[counter]] = 0;
					C[z] = 0;
					C[P[P[counter]]] = 1;
					counter = P[P[counter]];
					L[counter]=0; L[P[counter]]=0; L[RC[P[P[counter]]]]=0;
				}
				else {
					if(counter == RC[P[counter]]){
						counter = P[counter];
						leftRotate(RC,LC,P,counter,R);
					}
					C[P[counter]] = 0;
					C[P[P[counter]]] = 1;
					rightRotate(RC,LC,P,P[P[counter]],R);
					L[counter]=0; L[P[counter]]=0;  L[P[P[counter]]]=0; L[RC[P[P[counter]]]]=0;
				}	
			}
			else return 1;	
		}
		else if( L[counter]==0 && L[P[counter]]==0 && L[P[P[counter]]]==0 && L[LC[P[P[counter]]]]==0 ){
			__private int l1,l2,l3,l4;
			l1 = atomic_cmpxchg(&L[counter],0,1);
			l2 = atomic_cmpxchg(&L[P[counter]],0,1);
			l3 = atomic_cmpxchg(&L[P[P[counter]]],0,1);
			l4 = atomic_cmpxchg(&L[RC[P[P[counter]]]],0,1);
			//L[counter]=1; L[P[counter]]=1;  L[P[P[counter]]]=1; L[LC[P[P[counter]]]]=1;
			if(!l1 && !l2 && !l3 && !l4){
				__private int z = LC[P[P[counter]]];
				if(C[z]){
					C[P[counter]] = 0;
					C[z] = 0;
					C[P[P[counter]]] = 1;
					counter = P[P[counter]];
					L[counter]=0; L[P[counter]]=0; L[RC[P[P[counter]]]]=0;
				}
				else {
					if(counter == LC[P[counter]]){
						counter = P[counter];
						rightRotate(RC,LC,P,counter,R);
					}
					C[P[counter]] = 0;
					C[P[P[counter]]] = 1;
					leftRotate(RC,LC,P,P[P[counter]],R);
					L[counter]=0; L[P[counter]]=0;  L[P[P[counter]]]=0; L[RC[P[P[counter]]]]=0;
				}
			}
			else return 1;
		}	
	}
	C[R[0]] = 0;





		/*************************************************************************************/
		return 0;
	}
	else return 1;
	
}


/*void insertFixup(int index,__global int *LC,__global int *RC,__global int *P,__global int *C,__global int *L,__global int *R){
	
	while(C[P[index]]){
		if( (P[index] == LC[P[P[index]]]) && L[index]==0 && L[P[index]]==0 && L[P[P[index]]]==0 && L[RC[P[P[index]]]]==0 ){
			L[index]=1; L[P[index]]=1;  L[P[P[index]]]=1; L[RC[P[P[index]]]]=1;
			__private int y = RC[P[P[index]]];
			if(C[y]){
				C[P[index]] = 0;
				C[y] = 0;
				C[P[P[index]]] = 1;
				index = P[P[index]];
				L[index]=0; L[P[index]]=0; L[RC[P[P[index]]]]=0;
			}
			else {
				if(index == RC[P[index]]){
					index = P[index];
					leftRotate(RC,LC,P,index,R);
				}
				C[P[index]] = 0;
				C[P[P[index]]] = 1;
				rightRotate(RC,LC,P,P[P[index]],R);
				L[index]=0; L[P[index]]=0;  L[P[P[index]]]=0; L[RC[P[P[index]]]]=0;
			}		
		}
		else if( L[index]==0 && L[P[index]]==0 && L[P[P[index]]]==0 && L[LC[P[P[index]]]]==0 ){
			L[index]=1; L[P[index]]=1;  L[P[P[index]]]=1; L[LC[P[P[index]]]]=1;
			__private int y = LC[P[P[index]]];
			if(C[y]){
				C[P[index]] = 0;
				C[y] = 0;
				C[P[P[index]]] = 1;
				index = P[P[index]];
				L[index]=0; L[P[index]]=0; L[RC[P[P[index]]]]=0;
			}
			else {
				if(index == LC[P[index]]){
					index = P[index];
					rightRotate(RC,LC,P,index,R);
				}
				C[P[index]] = 0;
				C[P[P[index]]] = 1;
				leftRotate(RC,LC,P,P[P[index]],R);
				L[index]=0; L[P[index]]=0;  L[P[P[index]]]=0; L[RC[P[P[index]]]]=0;
			}
		}	
	}
	C[R[0]] = 0;
	
	return 0;
}*/



__kernel void rbtree(__global int* node, __global int* LC, __global int* RC, __global int* C, __global int* P, __global long * WL, __global int* L, __global int *counter, __global int *R, __global int *size)
{
	int base = get_global_id(0);
	if(base < size[0] && WL[base] != -1){
		__private int index;
		index = counter[0]+base;
		__private int choice;
		choice = RBInsert(WL[base],index,node,LC,RC,P,C,L,R);
		if(!choice)
			WL[base] = -1;
	}
	
}

__kernel void knl2(__global int* node, __global int* LC, __global int* RC, __global int* C, __global int* P, __global long * WL, __global int* L, __global int *counter, __global int *R, __global int *size){

	node[11] = 1111;

}

