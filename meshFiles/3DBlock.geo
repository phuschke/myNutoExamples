meshSize       = 0.5;

// create points
Point(1)  = { 0	,   0   ,   0,  meshSize}; 
Point(2)  = { 1	,   0   ,   0,  meshSize}; 
Point(3)  = { 1	,   1   ,   0,  meshSize}; 
Point(4)  = { 0	,	1   ,   0,  meshSize}; 



// create lines
l1  = newreg; Line(l1)  = {1,2};
l2  = newreg; Line(l2)  = {2,3};
l3  = newreg; Line(l3)  = {3,4};
l4  = newreg; Line(l4)  = {4,1};



loop1 = newll; Line Loop(loop1) = {l1,l2,l3,l4};


plane1 = news; Plane Surface(plane1) = {loop1};

Transfinite Surface {plane1};
Recombine Surface {plane1};

Extrude {0,0,1} 
{
Surface{plane1}; 
Layers{1/meshSize}; 
Recombine; 
}

Physical Volume(999) = {1:13};



