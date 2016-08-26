meshSize       = 100;
x_start = 0;
y_start = 0;
z_start = 0;

x_end = 10;
y_end = 10;
z_end = 10;

// create points
Point(1)  = {x_start    ,   y_start    	,   z_start,  meshSize}; 
Point(2)  = {x_end    	,   y_end    	,   z_end,  meshSize}; 



// create lines
l1  = newreg; Line(l1)  = {1,2};
Transfinite Line {l1} = 2;
Physical Line(999) = {l1};
