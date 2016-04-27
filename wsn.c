#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<string.h>
#include<errno.h>

char DAT_FILE[100]; /* locations of sensor nodes in the target
		       field to be stored in a data file */
int KEYRING_SIZE; /* key ring size m for each sensor node */
int KEYPOOL_SIZE; /* key pool size M */
int n; /* total number of sensor nodes to be deployed */
int d; /* average number of neighbor nodes for each sensor node */
int A; /*area of the sensor field in m^2*/
double RANGE; /* communication range of sensor node */
double p; /* network connectivity during direct key establishment phase */
double pi; /* network connectivity during path key establishment phase
	      using i hops in the path */
int avg_phynbr; //no of average physical nbrs of every node
int avg_keynbr; //no of average direct key nbrs of every node
int avg_pkeynbr; //no of average path direct key nbrs of every node
int *KEYPOOL;/*Keypool*/
int sensor_field[500][500][2]; /*Sensor Field containing the no of nodes with their ids present at a particular coordinate*/
/* a sensor node representation */
typedef struct {
	int id;
	int x; /* x-coordinate of sensor node location in target field */
	int y; /* y-coordinate of sensor node location in target field */
	int *keyring; /* key ring */
	int phynbrsize; /* number of physical neighbors of a sensor node */
	int keynbrsize; /* number of key neighbors of a sensor node */
	int pathkeynbrsize; /* number of path key neighbors of a sensor node */
	int phynbr[150][2]; /* list of physical neighbors of a sensor node */
	int keynbr[150]; /* list of key neighbors of a sensor node */
	int pathkeynbr[100]; /*list of path key negihbours*/
} sensor;
sensor *s;/*Set of sensors*/

void generate_sensor_data()
{
	int i, x, y;
	FILE *fp;
	memset(sensor_field, 0, sizeof(int)*250*250*2);
	fp = fopen("sensor.dat", "w+");
	n = 10000;
	fprintf(fp, "%d\n", n);
	for(i = 0; i < n; i++)
	{
		x = rand() % 500;
		y = rand() % 500;
		while(sensor_field[x][y][0] == 1)
		{
			x = rand() % 500;
			y = rand() % 500;
		}
		fprintf(fp, "%d %d\n", x, y);
		sensor_field[x][y][0] = 1;//mark presence of node
	}
	fclose(fp);
}

void get_sensors_count()
{
	FILE *fp;
	fp = fopen(DAT_FILE, "r");
	fscanf(fp, "%d", &n);
	fclose(fp);
}

void read_sensor_data()
{
	FILE *fp;
	int i, a, b;
	memset(sensor_field, 0, sizeof(int)*250*250*2);
	fp = fopen(DAT_FILE, "r");
	fscanf(fp, "%d", &n);
	for(i = 0; i < n; i++)
	{
		fscanf(fp, "%d %d", &a, &b);
		sensor_field[a][b][0] = 1;//presence of the sensor
		sensor_field[a][b][1] = i;//id of the sensor
		s[i].id = i;
		s[i].x = a;
		s[i].y = b;
	}
	fclose(fp);
}

void create_plot()
{
	FILE *fp;
	int i;
	char command[100];
	fp = fopen("sensors.in", "w+");
	for(i = 0; i < n; i++)
		fprintf(fp, "%d %d\n", s[i].x, s[i].y);
	fclose(fp);
	fp = fopen("gnuplot_script", "w+");
	fprintf(fp, "set term postscript;\n");
	fprintf(fp, "set output \"sensors_plot.ps\";\n");
	fprintf(fp, "plot \"sensors.in\" with points;\n");
	fclose(fp);
	strcpy(command, "gnuplot gnuplot_script; ps2pdf sensors_plot.ps; rm gnuplot_script sensors.in;");
	system(command);
}

void create_key_pool()
{
	/*choose a random number between 0-10000, then choose a random
	 * offset between 0-10000 and then add the offset to the num to get the random num
	 * , then just keep generating random offset between 0-10000 so adding it to the
	 * random number will give us a random number every time without repetition.
	 * This would cuase the range of keys to be from 0-100000000*/
	int ran_offset, i;
	KEYPOOL[0] = rand() % 10000;//first random key
	for(i = 1; i < KEYPOOL_SIZE; i++){
		ran_offset = rand() % 10000;
		KEYPOOL[i] = KEYPOOL[i - 1] + ran_offset;
	}
}

void create_key_rings()
{
	/*randomly choose index between 0-KEYPOOL_SIZE without repetition
	 * and allot that key to the key ring of a sensor*/
	int ran_index, i, j, check[KEYPOOL_SIZE];
	for(i = 0; i < n; i++)
	{
		memset(check, 0, sizeof(int)*KEYPOOL_SIZE);
		for(j = 0; j < KEYRING_SIZE; j++)
		{
			ran_index = rand() % KEYPOOL_SIZE;
			while(check[ran_index] == 1)
				ran_index = rand() % KEYPOOL_SIZE;
			check[ran_index] = 1;
			s[i].keyring[j] = KEYPOOL[ran_index];
		}
	}

}

void find_phy_neighbours()
{
	/*assuming that the range is square, so searching 
	 * for physical neighbours in 25m^2 area*/
	int i, a, b, node_id, count, avg = 0;
	for(i = 0; i < n; i++)
	{
		count = 0;
		int px, qx, py, qy;
		px = (s[i].x - 25)>0?(s[i].x - 25):0;
		qx = (s[i].x + 25)<500?(s[i].x + 25):499;
		py = (s[i].y - 25)>0?(s[i].y - 25):0;
		qy = (s[i].y + 25)<500?(s[i].y + 25):499;
		for(a = px; a <= qx; a++)
		{
			for(b = py; b <= qy; b++)
			{
				if((a == s[i].x) && (b == s[i].y))
					continue;
				if(sensor_field[a][b][0] == 1)
				{
					node_id = sensor_field[a][b][1];
					s[i].phynbr[count][0] = node_id;
					s[i].phynbr[count][1] = 0; //type of neighbour:0 for physical
					count++;
/*					if(count == d)
					{
						s[i].phynbr = (int*)realloc(s[i].phynbr, sizeof(int)*(d + 10));
					}
*/					//compare_keyrings(status, s[node_id1].keyring, s[node_id2].keyring);
				}
			}
		}
		s[i].phynbrsize = count;
		avg += count;
	}
	avg_phynbr = avg/n;
	printf("Average no of physical neighbours = %d\n", avg/n);

}

void compare_keyrings(int status[], int ring1[], int ring2[])
{
	int i, j;
	for(i = 0; i < status[0]; i++)
	{
		for(j = 0; j < status[1]; j++)
		{
			//printf("comparing i:%d j:%d s0:%d s1:%d\n", i, j, status[0], status[1]);
			if(ring1[i] == ring2[j])
			{
				status[0] = 1;
				return;
			}
		}
	}
	status[0] = 0;
}

void find_key_neighbours()
{
	int i, j, node_id1, node_id2, count, status[2], avg = 0;
	//status contains the two phynbr sizes
	//on return status[0] contains if the
	//two rings share a key or not
	for(i = 0; i < n; i++)
	{
		if(s[i].phynbrsize == 0)
			continue;
	/*	if(i >= 280)
		{
			printf("i:%d size:%d\n", i, s[i].phynbrsize);
			if((s[i].keynbr = (int*)malloc(sizeof(int)*s[i].phynbrsize)) == NULL)
			{
				printf("Out of Memory\n");
				printf("error:%s\n", strerror(errno));
				return ;
			}
		}
	*/	count = 0;
		node_id1 = s[i].id;
		for(j = 0; j < s[i].phynbrsize; j++)
		{
			node_id2 = s[i].phynbr[j][0];
			status[0] = s[i].phynbrsize;
			status[1] = s[node_id2].phynbrsize;
			//printf("s0:%d s1:%d\n", status[0], status[1]);
			compare_keyrings(status, s[node_id1].keyring, s[node_id2].keyring);
			if(status[0] == 1)
			{
				s[i].keynbr[count] = node_id2;
				s[i].phynbr[j][1] = 1;//1 for key nbr
				count++;
			}
		}
		s[i].keynbrsize = count;
		avg += count;
	}
	avg_keynbr = avg/n;
	printf("Average no of key neighbours = %d\n", avg/n);
}

int get_path(int cid, int nid, int hop)
{
	if(hop == 0)
		return 0;
	int i, j, c_nbrid;
	for(i = 0; i < s[cid].keynbrsize; i++)
	{
		c_nbrid = s[cid].keynbr[i];
		for(j = 0; j < s[c_nbrid].keynbrsize; j++)
		{
			if(s[c_nbrid].keynbr[j] == nid)
				return 1;
		}
	}
	for(i = 0; i < s[cid].keynbrsize; i++)
	{
		if(get_path(s[cid].keynbr[i], nid, hop - 1) == 1)
			return 1;
	}
	return 0;
}

void find_path_neighbours(int hop)
{
	int i, j, op, count, avg = 0;
	for(i = 0; i < n; i++)
	{
		count = 0;
		for(j = 0; j < s[i].phynbrsize; j++)
		{
			if(s[i].phynbr[j][1] == 1)
				continue;
			op = get_path(i, s[i].phynbr[j][0], hop); //find out a path between s[i] and its non-key-phy-nbrs
			if(op == 1)
			{
				s[i].pathkeynbr[count] = s[i].phynbr[j][0];
				count += 1;
				s[i].phynbr[j][1] = 2;
			}
		}
		s[i].pathkeynbrsize = count;
		avg += count;
	}
	avg_pkeynbr = avg/n;
	printf("Average no path key neighbours = %d\n", avg/n);
}

float compute_connectivity(int hop)
{
	float total = 1;
	if(hop == 0)
	{
		int i;
		float kr, kp;
		kp = (float)KEYPOOL_SIZE;
		kr = (float)KEYRING_SIZE;
		for(i = 0; i < KEYRING_SIZE; i++)
			total *= ((kp - kr - i) / (kp - i));
		total = 1 - total;
		return total;
	}
	float t, t1;
	t = compute_connectivity(hop - 1);
	t1 = compute_connectivity(0);
	total = 1 - ((1 - t) * pow((1 - (t * t1)), d));
	printf("hop:%d connectivity:%f\n", hop, total);
	return total;
}

void store_data()
{
	//File format
	//Key-pool size
	//key-pool array
	//Key-ring size
	//RANGE
	//n
	//sensor-id x y
	//keyring array
	//phynbrsize
	//phynbr array
	//keynbrsize
	//keynbr array
	//pathkeynbrsize
	//pathkeynbr array
	int i, j;
	FILE *fp;
	fp = fopen("sensor.out", "w+");
	fprintf(fp, "%d\n", n);
	fprintf(fp, "%d\n", KEYPOOL_SIZE);
	for(i = 0; i < KEYPOOL_SIZE; i++)
		fprintf(fp, "%d ", KEYPOOL[i]);
	fprintf(fp, "\n");
	fprintf(fp, "%d\n", KEYRING_SIZE);
	fprintf(fp, "%lf\n", RANGE);
	for(i = 0; i < n; i++)
	{
		fprintf(fp, "%d %d %d\n",s[i].id, s[i].x, s[i].y);
		for(j = 0; j < KEYRING_SIZE; j++)
			fprintf(fp, "%d ", s[i].keyring[j]);
		fprintf(fp, "\n");
		fprintf(fp, "%d\n", s[i].phynbrsize);
		for(j = 0; j < s[i].phynbrsize; j++)
			fprintf(fp, "%d ", s[i].phynbr[j][0]);
		fprintf(fp, "\n");
		fprintf(fp, "%d\n", s[i].keynbrsize);
		for(j = 0; j < s[i].keynbrsize; j++)
			fprintf(fp, "%d ", s[i].keynbr[j]);
		fprintf(fp, "\n");
		fprintf(fp, "%d\n", s[i].pathkeynbrsize);
		for(j = 0; j < s[i].pathkeynbrsize; j++)
			fprintf(fp, "%d ", s[i].pathkeynbr[j]);
		fprintf(fp, "\n");
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	/*********Initialise data**************/
	strcpy(DAT_FILE, argv[1]);
	KEYRING_SIZE = atoi(argv[2]);//100
	KEYPOOL_SIZE = atoi(argv[3]);//40000
	KEYPOOL = (int*)malloc(sizeof(int)*KEYPOOL_SIZE);
	A = 250000;
	int hop = 2;
	RANGE = 25;
	//generate_sensor_data();
	get_sensors_count();
	printf("n:%d\n", n);
	d = (n*M_PI*RANGE*RANGE/A) - 1;
	/*********Initialise data**************/

	/********Task 1**************/
	printf("Initiating Task-1\n");
	s = (sensor *)malloc(sizeof(sensor)*n);
	int i;
	//allocate memory for keyrings
	//need to allocate here, since in function create_key_rings, segfault occurs due to memory leak
	for(i = 0; i < n; i++)
	{
		if((s[i].keyring = (int*)malloc(sizeof(int)*KEYRING_SIZE)) == NULL)
		{
			printf("Out of Memory\n");
			printf("error:%s\n", strerror(errno));
			return 0;
		}
	}
	//allocate memory for phy nbrs
	//need to allocate here, since in function find_phy_neighbours, segfault occurs due to memory leak
/*	for(i = 0; i < n; i++)
	{
		if((s[i].phynbr = (int*)malloc(sizeof(int)*d)) == NULL)
		{
			printf("Out of Memory\n");
			printf("error:%s\n", strerror(errno));
			return 0;
		}
	}
*/	printf("Reading Data File...\n");
	read_sensor_data();
	printf("Plotting Sensors...\n");
	create_plot();
	printf("Task-1 Successfully Completed\n\n");
	/********Task 1**************/

	/*******Key Pre-distribution Phase*******/
	printf("Initiating Key Pre-Dristribution Phase\n");
	printf("Generating Key-Pool...\n");
	create_key_pool();
	printf("Key-Pool generated\n");
	printf("Generating Key-Rings for each sensor node...\n");
	create_key_rings();
	printf("Key-Rings generated\n");
	printf("Key Pre-Dristribution Phase Successfully Completed\n\n");
	/*******Key Pre-distribution Phase*******/

	/********Task 2 - Direct Key Establishment**************/
	printf("Initiating Task-2 i.e. Direct-Key Establishment Phase\n");
	printf("Searching for Physical Neighbours\n");
	find_phy_neighbours();
	printf("Physical Neighbours Located\n");
	printf("Negotiating Keys With Physical Neighbours. It will take a while...\n");
	//allocate memory for key nbrs
	//need to allocate here, since in function find_key_neighbours, segfault occurs due to memory leak
/*	for(i = 0; i < 280; i++)
	{
		//printf("i:%d size:%d\n", i, s[i].phynbrsize);
		if((s[i].keynbr = (int*)malloc(sizeof(int)*s[i].phynbrsize)) == NULL)
		{
			printf("Out of Memory\n");
			printf("error:%s\n", strerror(errno));
			return 0;
		}
	}
*/	find_key_neighbours();
	printf("Key Negotiation Complete\n");
	printf("Simualted Connectivity: %f\n", (float)((float)(avg_keynbr)/(float)(avg_phynbr)));
	printf("Theoritical Connectivity: %f\n", compute_connectivity(0));
	printf("Task-2 i.e. Direct-Key Establishment Phase Successfuly Completed\n\n");
	/********Task 2 - Direct Key Establishment**************/

	/********Task 3 - Path Key Establishment**************/
	printf("Initiating Task-3 i.e. Path-Key Establishment Phase\n");
	find_path_neighbours(hop);
	printf("Path Key Negotiation Complete\n");
	printf("Simualted Connectivity: %f\n", ((float)(avg_pkeynbr + avg_keynbr))/((float)(avg_phynbr)));
	printf("Theoritical Connectivity: %f\n", compute_connectivity(2));
	printf("Task-3 i.e. Path-Key Establishment Phase Successfuly Completed\n\n");
	/********Task 3 - Path Key Establishment**************/

	/****************Write all sensor data***************/
	/*printf("Storing all sensor data in sensor.out\n");
	store_data();
	*//****************Write all sensor data***************/

	return 0;
}
