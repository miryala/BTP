#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif	
 
#define MAX_SOURCE_SIZE (0x100000)

void insertFixup(int index,int* LC,int *RC,int *P,int *C,int *R);

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

void leftRotate(int *RC,int *LC,int *P,int x,int *R){
	int y;
	y = RC[x];
	RC[x] = LC[y];
	if( LC[y] != 0)
		P[LC[y]] = x;
	P[y] = P[x];
	if(P[x] == 0){
		P[y] = 0;
		*R = y;
		}
	else if(x == LC[P[x]])
		LC[P[x]] = y;
	else RC[P[x]] = y;
	LC[y] = x;
	P[x] = y;
}

void rightRotate(int *RC,int *LC,int *P,int y,int *R){
	int x;
	x = LC[y];
	LC[y] = RC[x];
	if( RC[x] != 0)
		P[RC[x]] = y;
	P[x] = P[y];
	if(P[y] == 0){
		P[x] = 0;
		*R = x;
		}
	else if(y == LC[P[y]])
		LC[P[y]] = x;
	else RC[P[y]] = x;
	RC[x] = y;
	P[y] = x;
}

void RBInsert(long value,int counter,int *node,int *LC,int *RC,int *P,int *C,int *R){
	
	int x,y;
	x=*R;
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
	if ( y == 0 ){
		P[counter] = 0;
		*R = counter;
		}
	else if( value < node[y])
		LC[y] = counter;
	else RC[y] = counter;
	insertFixup(counter,LC,RC,P,C,R);
}

void insertFixup(int index,int* LC,int *RC,int *P,int *C,int *R){
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
					leftRotate(RC,LC,P,index,R);
				}
				C[P[index]] = 0;
				C[P[P[index]]] = 1;
				rightRotate(RC,LC,P,P[P[index]],R);
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
					rightRotate(RC,LC,P,index,R);
				}
				C[P[index]] = 0;
				C[P[P[index]]] = 1;
				leftRotate(RC,LC,P,P[P[index]],R);
			}
		}
	}
	C[*R] = 0;
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
	//printf("%c \n",c);
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
	int per = 1;
	int time1=0;
	printf("X	Y\n");
	while(1){
	printf("%d	",per);
	n = (i/100)*per;
	int counter = 1;
	int node[i+n+1];
	int LC[i+n+1];
	int RC[i+n+1];
	int P[i+n+1];
	int C[i+n+1];
	int L[i+n+1];
	int trackList[n];
	int R = 1;
	for (j=0; j<i+n+1; j++){
		node[j] = -1;
		LC[j] = 0;
		RC[j] = 0;
		P[j] = -1;
		C[j] = 1;
		L[j] = 0;
	}
	C[0] = 0;
	long value = 0;
	clock_t t;
        t = clock();
	/* creating initial tree */
	for (j=0; j<i; j++){
		value = rand_range_re(&seed,r);
		RBInsert(value,counter,node,LC,RC,P,C,&R);
		counter++;
	}
	t = clock() - t;
        double time_taken = ((double)t); // in seconds
	//printf(" %f micro seconds to execute \n", time_taken);
	time1 += time_taken;
	/*for (j=1;j<i+1;j++){
		printf( "j:%d node:%d LC:%d RC:%d P:%d C:%d\n",j,node[j],LC[j],RC[j],P[j],C[j]);	
	}*/

	long WL[n];
	for(j=0;j<n;j++){
		value =  rand_range_re(&seed,r);
		WL[j] = value;
		trackList[j] = 0;
		//printf("%ld   ",value);
	}
/************************************************************* OpenCL ************************************************************************************/

	cl_platform_id *platform_id = malloc(2*sizeof(cl_platform_id));
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem nodemobj = NULL;	
	cl_mem LCmobj = NULL;	
	cl_mem RCmobj = NULL;	
	cl_mem Cmobj = NULL;
	cl_mem Pmobj = NULL;
	cl_mem WLmobj = NULL;
	cl_mem Lmobj = NULL;
	cl_mem Cntmobj = NULL;
	cl_mem Rmobj = NULL;
	cl_mem sizemobj = NULL;
	cl_mem trackmobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_program program2 = NULL;
	cl_kernel kernel2 = NULL;	
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;
	cl_build_status status;

	size_t logSize;
 	int root[1];
	int cnt[1]; 
	int size[1];
	root[0] = R;
	cnt[0] = counter;
	size[0] = n;
	FILE *fp;
	const char fileName[] = "./rbtree.cl";
	size_t source_size;
	char *source_str;
 	char *programLog;
	/* Load kernel source file */
	fp = fopen(fileName, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");	
		exit(1);
	}	
	source_str = (char *)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
	
 
	/* Get Platform/Device Information*/
	ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);	
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
 
	/* Create OpenCL Context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
 
	/* Create command queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
 
	/* Create Buffer Object */
	nodemobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (i+n+1)*sizeof(int), NULL, &ret);
	LCmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (i+n+1)*sizeof(int), NULL, &ret);
	RCmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (i+n+1)*sizeof(int), NULL, &ret);
	Cmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (i+n+1)*sizeof(int), NULL, &ret);
	Pmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (i+n+1)*sizeof(int), NULL, &ret);
	WLmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (n)*sizeof(long), NULL, &ret);
	Lmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, (i+n+1)*sizeof(int), NULL, &ret);
	Cntmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret);
	Rmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret);
	sizemobj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret);
	trackmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(int), NULL, &ret);
 
 
	/* Copy input data to the memory buffer */
	ret = clEnqueueWriteBuffer(command_queue, nodemobj, CL_TRUE, 0, (i+n+1)*sizeof(int), node, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, LCmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), LC, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, RCmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), RC, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, Cmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), C, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, Pmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), P, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, WLmobj, CL_TRUE, 0, (n)*sizeof(long), WL, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, Lmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), L, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, Cntmobj, CL_TRUE, 0, sizeof(int), cnt, 0, NULL, NULL); 
	ret = clEnqueueWriteBuffer(command_queue, Rmobj, CL_TRUE, 0, sizeof(int), root, 0, NULL, NULL); 
	ret = clEnqueueWriteBuffer(command_queue, sizemobj, CL_TRUE, 0, sizeof(int), size, 0, NULL, NULL); 
	ret = clEnqueueWriteBuffer(command_queue, trackmobj, CL_TRUE, 0, n*sizeof(int), &trackList, 0, NULL, NULL);

	/* Create kernel program from source file */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);	
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
 	if (ret != CL_SUCCESS) {
 
        // check build error and build status first
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, 
                sizeof(cl_build_status), &status, NULL);
 
        // check build log
        clGetProgramBuildInfo(program, device_id, 
                CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        programLog = (char*) calloc (logSize+1, sizeof(char));
        clGetProgramBuildInfo(program, device_id, 
                CL_PROGRAM_BUILD_LOG, logSize+1, programLog, NULL);
        printf("Build failed; error=%d, status=%d, programLog:nn%s", 
                ret, status, programLog);
        free(programLog);
 
    }
	/* Create data parallel OpenCL kernel */	
	kernel = clCreateKernel(program, "rbtree", &ret);
	program2 = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	ret = clBuildProgram(program2, 1, &device_id, NULL, NULL, NULL);
	if (ret != CL_SUCCESS) {
 
        // check build error and build status first
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, 
                sizeof(cl_build_status), &status, NULL);
 
        // check build log
        clGetProgramBuildInfo(program, device_id, 
                CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        programLog = (char*) calloc (logSize+1, sizeof(char));
        clGetProgramBuildInfo(program, device_id, 
                CL_PROGRAM_BUILD_LOG, logSize+1, programLog, NULL);
        printf("Build failed; error=%d, status=%d, programLog:nn%s", 
                ret, status, programLog);
        free(programLog);
 
    }
	kernel2 = clCreateKernel(program2, "knl2", &ret);
	//printf("ret val:%d\n",ret);
	/* Set OpenCL kernel arguments */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&nodemobj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&LCmobj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&RCmobj);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&Cmobj);
	ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&Pmobj);
	ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&WLmobj);
	ret = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *)&Lmobj);
	ret = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&Cntmobj);
	ret = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void *)&Rmobj);
	ret = clSetKernelArg(kernel, 9, sizeof(cl_mem), (void *)&sizemobj);
 	ret = clSetKernelArg(kernel, 10, sizeof(cl_mem), (void *)&trackmobj);
	size_t local_item_size = 64;
	size_t global_item_size = (n/64+1)*64;
			
	/* Execute OpenCL kernel as data parallel */
	clock_t t1;
    t1 = clock();
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
	&global_item_size, &local_item_size, 0, NULL, NULL);
 	ret = clFinish(command_queue);
	int count = 0;
	int extra = 0;
	while(1){
		count++;
		ret = clSetKernelArg(kernel2, 0, sizeof(cl_mem), (void *)&nodemobj);
		ret = clSetKernelArg(kernel2, 1, sizeof(cl_mem), (void *)&LCmobj);
		ret = clSetKernelArg(kernel2, 2, sizeof(cl_mem), (void *)&RCmobj);
		ret = clSetKernelArg(kernel2, 3, sizeof(cl_mem), (void *)&Cmobj);
		ret = clSetKernelArg(kernel2, 4, sizeof(cl_mem), (void *)&Pmobj);
		ret = clSetKernelArg(kernel2, 5, sizeof(cl_mem), (void *)&WLmobj);
		ret = clSetKernelArg(kernel2, 6, sizeof(cl_mem), (void *)&Lmobj);
		ret = clSetKernelArg(kernel2, 7, sizeof(cl_mem), (void *)&Cntmobj);
		ret = clSetKernelArg(kernel2, 8, sizeof(cl_mem), (void *)&Rmobj);
		ret = clSetKernelArg(kernel2, 9, sizeof(cl_mem), (void *)&sizemobj);
		ret = clSetKernelArg(kernel2, 10, sizeof(cl_mem), (void *)&trackmobj);
		local_item_size = 64;
		global_item_size = (n/64+1)*64;
		ret = clEnqueueNDRangeKernel(command_queue, kernel2, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
		ret = clFinish(command_queue);
		ret = clEnqueueReadBuffer(command_queue, WLmobj, CL_TRUE, 0, (n)*sizeof(long), WL, 0, NULL, NULL);
		int flag = 1;
		clock_t t3;
		t3 = clock();
		for(j=0;j<n;j++){
			if(WL[j] != -1){
				flag = 0;
				break;
			}
		}
		t3 = clock() - t3;
		extra += t3;
		if(flag == 1 || count > 52)
			break;
		//printf("iteration count: %d",count);
	}
	t1 = clock() - t1;
    double time_taken2 = ((double)t1); // in seconds
	/* Transfer result to host */
	long w[n];
	ret = clEnqueueReadBuffer(command_queue, nodemobj, CL_TRUE, 0, (i+n+1)*sizeof(int), node, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, WLmobj, CL_TRUE, 0, (n)*sizeof(long), w, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, LCmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), LC, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, Cmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), C, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, Pmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), P, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, RCmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), RC, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, Lmobj, CL_TRUE, 0, (i+n+1)*sizeof(int), L, 0, NULL, NULL);
	ret = clEnqueueReadBuffer(command_queue, trackmobj, CL_TRUE, 0, (n)*sizeof(int), trackList, 0, NULL, NULL);
	
 	/*for (j=1;j<i+n+1;j++){
		printf( "j:%d node:%d\n",j,node[j]	);	
	}*/
	/*for (j=0;j<i+n+1;j++){
		printf( "j:%d node:%d LC:%d RC:%d P:%d C:%d L:%d\n",j,node[j],LC[j],RC[j],P[j],C[j],L[j]);	
	}
 	for(j=0;j<n;j++)
	printf("WL:%ld--TL:%d  ",w[j],trackList[j]);*/
	printf("%f\n", time_taken2);
	/* Finalization */
	ret = clFlush(command_queue);	
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(nodemobj);
	ret = clReleaseMemObject(LCmobj);
	ret = clReleaseMemObject(RCmobj);
	ret = clReleaseMemObject(Cmobj);
	ret = clReleaseMemObject(Pmobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);	
 
	free(source_str); 
	if(per>50)
		break;
	per++;
	}
/*********************************************************************************************/
return 0;
}
