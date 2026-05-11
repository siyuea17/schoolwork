#include <iostream>
#include <omp.h>

using namespace std;

int main()
{
	omp_set_num_threads(12);
#pragma omp parallel
	{
#pragma omp single
		{
			cout << "single thread=" << omp_get_thread_num() << endl;
		}
		cout << omp_get_thread_num() << endl;
	}

}