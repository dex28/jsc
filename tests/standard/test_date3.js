
d1 = new Date(1998,0,3,12,30,5);
d2 = new Date(1998,0,4,12,30,5);
d3 = new Date(1998,1,3,12,30,5);

print( d1, d1.getTime () );
print( d2, d2.getTime () );
print( d3, d3.getTime () );

print( d1 < d2, d1.getTime() < d2.getTime () );
print( d1 >= d2, d1.getTime () >= d2.getTime () );
print( d3 < d1, d3.getTime () < d1.getTime () );

