void main(int a, int b, int c)
/*@ predicate p : a > 2 */
/*@ predicate q : b <= 2 */
/*@ predicate r : b <= 0 */
{
a=3;		/*@ abstraction p = true; */
b=3;		/*@ abstraction q = false; r = false; */ 
c=a*2-b;	/*@ abstraction skip */
while(b>0)	/*@ abstraction while(!r) */ 
{
if(a<=b)	/*@ abstraction if((p&&q)?(false):(((!p)&&(!q))?(true):(*))) */
{
    a=4;	/*@ abstraction p = true; */
}
else
{
    a=a-b-1;	/*@ abstraction p = ((!p)&&(!r))?(false):(*); */
}
b=b-2;		/*@ abstraction q = (q)?(true):(*); r = q; */
c=c+b+a;	/*@ abstraction skip */
}
assert(a>2); 	/*@ abstraction assert(p) */
}

