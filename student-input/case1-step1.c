void main(int a, int b, int c)
/*@ predicate p : a > 2 */
{
a=3;		/*@ abstraction p = true; */
b=3;		/*@ abstraction skip */ 
c=a*2-b;	/*@ abstraction skip */
while(b>0)	/*@ abstraction while(*) */ 
{
if(a<=b)	/*@ abstraction if(*) */
{
    a=4;	/*@ abstraction p = true; */
}
else
{
    a=a-b-1;	/*@ abstraction p = *; */
}
b=b-2;		/*@ abstraction skip */
c=c+b+a;	/*@ abstraction skip */
}
assert(a>2); 	/*@ abstraction assert(p==true) */
}

