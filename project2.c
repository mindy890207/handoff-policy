#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
struct car
{
	int x, y;
	int direction[4]; // =>, <=, ^, V
	int server;		  /*server num*/
	struct car *last; /*car list*/
	struct car *next;
};

double delta = 1;
double carEnterProb;
struct car *cars = NULL;
int V = 10; /*velocity*/
int Pt = 100;
int Pmin = 10;
float powerAvgMin = 17.894217; //calculate by code
float powerAvgMax;
float policy(int, double, double, int, int, double);
int move(struct car **, int);
int setServer(struct car **, int, int);
int generateCar(struct car **, int, double);

int main()
{
	Pt = 100;
	Pmin = 10;
	double lamda = (double)1 / (double)2; /*change lambda*/
	srand(time(NULL));

	float power[4];
	power[0] = policy(Pt, 0, 0, Pmin, 0, lamda); /*Pt,E,T,Pmin,0*/
	powerAvgMax = power[0];
	float x = (powerAvgMax + powerAvgMin) * (0.5);
	printf("best policy : %f\n", power[0]);
	power[1] = policy(Pt, 14.25, 0, Pmin, 1, lamda); //T
	if (power[1] > x)
	{
		printf("Threshold policy : %f\n", power[1]);
	};
	power[2] = policy(Pt, 0, 11.5, Pmin, 2, lamda); //E
	if (power[2] > x)
	{
		printf("Entropy policy : %f\n", power[2]);
	};

	power[3] = policy(Pt, 0, 11.1, Pmin, 3, lamda);
	if (power[3] > x)
	{
		printf("my policy : %f\n", power[3]);
	};
	return 0;
}
float policy(int Pt, double T, double E, int Pmin, int method, double lamda)
{
	int handoffCount = 0;
	float totalPower = 0;
	int carCount = 0;
	struct car *lastCar = NULL;

	FILE *file;
	switch (method)
	{
	case 0:
		file = fopen("bestPolicy.txt", "w");
		break;
	case 1:
		file = fopen("thresholdPolicy.txt", "w");
		break;
	case 2:
		file = fopen("entropyPolicy.txt", "w");
		break;
	case 3:
		file = fopen("myPolicy.txt", "w");
		break;
	}

	for (int i = 0; i < 86400; i++)
	{
		int localhandoff = 0;
		carCount = move(&lastCar, carCount);
		struct car *nowCar = cars;
		float nowTotalPower = 0;
		while (nowCar)
		{ // handoff
			float power[4], distance[4];
			int maxPower = nowCar->server - 1; //server1:power[0]
			int serverX[4] = {330, 640, 360, 660};
			int serverY[4] = {350, 310, 680, 658};
			for (int j = 0; j < 4; ++j)
			{
				distance[j] = sqrt(pow(nowCar->x - serverX[j], 2) + pow(nowCar->y - serverY[j], 2));
				power[j] = Pt - 33;
				if (distance[j] > 1)
					power[j] -= 20 * log10(distance[j]);
			}
			float Pold = power[maxPower];
			for (int j = 0; j < 4; ++j)
			{
				if ((power[j] > power[maxPower] && method == 0 /*&& Pold < Pmin:算AVGMIN*/) ||
					(power[j] > power[maxPower] && Pold < T && method == 1) ||
					(power[j] > power[maxPower] && power[j] > Pold + E && method == 2 && i != 0))
				{
					maxPower = j;
				}
			}
			if (method == 3)
			{
				if (nowCar->x <= 375 && nowCar->y <= 475)
				{
					if (power[0] > power[maxPower] && power[0] > Pold + E && i != 0)
					{
						maxPower = 0;
					}
				}
				else if (nowCar->x > 475 && nowCar->y <= 425)
				{
					if (power[1] > power[maxPower] && power[1] > Pold + E && i != 0)
					{
						maxPower = 1;
					}
				}
				else if (nowCar->x <= 475 && nowCar->y > 575)
				{
					if (power[2] > power[maxPower] && power[2] > Pold + E && i != 0)
					{
						maxPower = 2;
					}
				}
				else if (nowCar->x > 575 && nowCar->y > 525)
				{
					if (power[3] > power[maxPower] && power[3] > Pold + E && i != 0)
					{
						maxPower = 3;
					}
				}
			}
			if (nowCar->server - 1 != maxPower)
			{
				++handoffCount;
				++localhandoff;
				nowCar->server = maxPower + 1;
			}
			nowTotalPower += power[maxPower];
			nowCar = nowCar->next;
		}
		if (carCount != 0)
			totalPower += (nowTotalPower / carCount);
		fprintf(file, "%d %d\n", i, localhandoff);
		carCount = generateCar(&lastCar, carCount, lamda);
	}
	fclose(file);
	switch (method)
	{
	case 0:
		printf("best policy: %d\n", handoffCount);
		break;
	case 1:
		printf("threshold policy: %d\n", handoffCount);
		break;
	case 2:
		printf("entropy policy: %d\n", handoffCount);
		break;
	case 3:
		printf("my policy: %d\n", handoffCount);
		break;
	}

	return totalPower / 86400.;
}

int move(struct car **lastCar, int carCount)
{
	struct car *nowCar = cars;
	while (nowCar)
	{
		if (nowCar->x % 100 == 0 && nowCar->y % 100 == 0)
		{ // at corner

			if (nowCar->x == 0 && nowCar->y == 0)
			{ // (0,0)

				if (nowCar->direction[1])
				{ // direction <=
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[2] = 1; //turn to ^
				}
				else if (nowCar->direction[3])
				{ // direction V
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[0] = 1; //turn to =>
				}
			}
			else if (nowCar->x == 1000 && nowCar->y == 0)
			{ //(1000,0)

				if (nowCar->direction[0])
				{ // direction =>
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[2] = 1; //turn to^
				}
				else if (nowCar->direction[3])
				{ //direction V
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[1] = 1; //turn to <=
				}
			}
			else if (nowCar->x == 0 && nowCar->y == 1000)
			{ // (0,1000)

				if (nowCar->direction[1])
				{ // direction <=
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[3] = 1; //turn to V
				}
				else if (nowCar->direction[2])
				{ // direction ^
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[0] = 1; //turn to =>
				}
			}
			else if (nowCar->x == 1000 && nowCar->y == 1000)
			{ // (1000,1000)

				if (nowCar->direction[0])
				{ // direction =>
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[3] = 1; //turn to V
				}
				else if (nowCar->direction[2])
				{ // direction ^
					for (int i = 0; i < 4; ++i)
						nowCar->direction[i] = 0;
					nowCar->direction[1] = 1; //turn to <=
				}
			}
			else
			{

				int newDirection = rand() % 5 + 1; //1~3:straight,4:right,5:left
				if (newDirection == 4)
				{ // 4:right

					if (nowCar->direction[0])
					{ // direction =>
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[3] = 1; //direction V
					}
					else if (nowCar->direction[1])
					{ // direction <=
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[2] = 1; // direction ^
					}
					else if (nowCar->direction[2])
					{ // direction ^
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[0] = 1; // direction =>
					}
					else if (nowCar->direction[3])
					{ //direction V
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[1] = 1; // direction <=
					}
				}
				else if (newDirection == 5)
				{ // 5:left

					if (nowCar->direction[0])
					{ // direction =>
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[2] = 1; // direction ^
					}
					else if (nowCar->direction[1])
					{ // direction <=
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[3] = 1; //direction V
					}
					else if (nowCar->direction[2])
					{ // direction ^
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[1] = 1; // direction <=
					}
					else if (nowCar->direction[3])
					{ //direction V
						for (int i = 0; i < 4; ++i)
							nowCar->direction[i] = 0;
						nowCar->direction[0] = 1; // direction =>
					}
				}
			}
		}
		nowCar->x += (V * nowCar->direction[0]);
		nowCar->x -= (V * nowCar->direction[1]);
		nowCar->y += (V * nowCar->direction[2]);
		nowCar->y -= (V * nowCar->direction[3]);
		nowCar = nowCar->next;
	}

	nowCar = *lastCar;
	bool flag = 1;
	int departcount = 0;
	while (nowCar)
	{
		if (nowCar->x < 0 || nowCar->x > 1000 || nowCar->y < 0 || nowCar->y > 1000)
		{
			if (flag)
			{
				*lastCar = (*lastCar)->last;
			}
			if (nowCar->last)
			{
				nowCar->last->next = nowCar->next;
			}
			else
			{
				cars = nowCar->next;
			}
			if (nowCar->next)
			{
				nowCar->next->last = nowCar->last;
			}
			struct car *temp = nowCar->last;
			free(nowCar);
			nowCar = temp;
			--carCount;
			departcount++;
		}
		else
		{
			nowCar = nowCar->last;
			flag = 0;
		}
	}
	//printf("departcount = %d\n", departcount);

	return carCount;
}

int setServer(struct car **newCar, int x, int y)
{
	int server = 1;
	float power[4], distance[4];
	int serverX[4] = {330, 640, 360, 660};
	int serverY[4] = {350, 310, 680, 658};
	int temp;
	for (int j = 0; j < 4; j++)
	{

		distance[j] = sqrt(pow(x - serverX[j], 2) + pow(y - serverY[j], 2));
		//printf("distance[%d] = %f\n",j,distance[j]);
		if (distance[j] > 1)
			power[j] = Pt - (33 + 20 * log10(distance[j]));
		if (j == 0)
		{
			temp = power[0];
		}
		if (j > 0)
		{
			if (power[j] > temp)
			{
				server = j + 1;
				temp = power[j];
			}
		}

		//printf("power[%d]=%f\n",j+1,power[j]);
	}
	return server;
}
int generateCar(struct car **lastCar, int carCount, double lamda)
{
	for (int i = 0; i < 36; i++)
	{
		carEnterProb = 1 - exp(-lamda * delta);
		double s = (double)rand() / (RAND_MAX + 1.0);
		if (s < carEnterProb)
		{

			struct car *newCar = malloc(sizeof(struct car));
			if (!*lastCar)
			{ // first car
				cars = newCar;
				newCar->last = NULL;
				newCar->next = NULL;
				*lastCar = newCar;
			}
			else
			{
				(*lastCar)->next = newCar;
				newCar->last = *lastCar;
				newCar->next = NULL;
				*lastCar = newCar;
			}

			if (i < 9)
			{ /*  =>*/
				for (int i = 0; i < 4; i++)
					newCar->direction[i] = 0;
				newCar->direction[0] = 1;
				newCar->x = 0;
				newCar->y = 100 * (i % 10 + 1);
			}
			else if (i < 18)
			{ /*  <=*/
				for (int i = 0; i < 4; i++)
					newCar->direction[i] = 0;
				newCar->direction[1] = 1;
				newCar->x = 1000;
				newCar->y = 100 * ((i + 1) % 10 + 1);
			}
			else if (i < 27)
			{ /* ^ */
				for (int i = 0; i < 4; i++)
					newCar->direction[i] = 0;
				newCar->direction[2] = 1;
				newCar->x = 100 * ((i + 2) % 10 + 1);
				newCar->y = 0;
			}
			else
			{ /*  V */
				for (int i = 0; i < 4; i++)
					newCar->direction[i] = 0;
				newCar->direction[3] = 1;
				newCar->x = 100 * ((i + 3) % 10 + 1);
				newCar->y = 1000;
			}

			// set server

			newCar->server = setServer(&newCar, newCar->x, newCar->y);

			carCount++;
			//printf("set %d ,(x,y)=(%d,%d),server:%d\n", i, newCar->x, newCar->y, newCar->server);
		}
	}
	//printf("carCount:%d\n", carCount);
	return carCount;
}
