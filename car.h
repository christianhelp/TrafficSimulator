#ifndef _car_h
#define _car_h

typedef struct Car
{
    /*TODO:  Add information to store about cars*/
    
    /*Start and dest of this one is a vertices*/
    int start;
    int dest;
    
    int stepAdded;
    /*Set to whatever the time step is if it has moved during that time step*/
    int hasMoved;
    

}  Car;

#endif
