#include<iostream>
using namespace std;
class employee
{
	public:
	int reg;
	string name;
	string des;
	float salary;
	public:
		void getinfo()
		{
			cout<<"Enter Registration no: "<<endl;
			cin>>reg;
			cout<<"Enter Name: "<<endl;
			cin>>name;
			cout<<"Enter Destination: "<<endl;
			cin>>des;
			cout<<"Enter basic salary: "<<endl;
			cin>>salary;
		}
		
};
class salary:public employee
{
	public:
		float salary;
	float fund;
	float sum;
		float getsalary()
		{
			fund=(salary/100)*10;
			return fund;
		}
		void calculatesum()
		{
			sum=salary+fund;
			cout<<"Total salary: "<<sum<<endl;
		}
};
int main()
{
	salary s1;
s1.getinfo();
s1.getsalary();
s1.calculatesum();
}