#include "trafficSimulator.h"

/* Function declarations: */

TrafficData* readTrafficDataFromInputFile( char* name );
void printRoadData(TrafficData* traffic,Event* event);
void freeData(TrafficData* traffic);
void updateLights(TrafficData* traffic);
void freeAllThatShii(TrafficData* tData);
/* printNames
 * input: none
 * output: none
 *
 * Prints names of the students who worked on this solution
 */
void printNames( )
{
    /* TODO : Fill in you and your partner's names (or N/A if you worked individually) */
    printf("This solution was completed by:\n");
    printf("Christian Walker\n");
    printf("Lauren Grissom\n");
}


/* trafficSimulator
 * input: char* name of file to read
 * output: N/A
 *
 * Read in the traffic data from the file whose name is stored in char* filename
 * Simulate the rules outlined in the project 2 description
 */
void trafficSimulator( char* filename ){
    /* TODO: complete this function */

    bool gridlock = false;
    Event* event;
    int i,j,nextShortest = -1;
    /*Checks if anything has moved and should be set back to 0 if something has moved*/
    int hasAnythingMoved = 0;
    int avgSteps = 0;
    int longestMove = 0;
    int totalCars = 0;
    
    /* Read in the traffic data from filename.  It's recommended you call readTrafficDataFromInputFile to do this: */
    TrafficData* traffic = readTrafficDataFromInputFile( filename );
    
    
    do{
        updateLights(traffic);
        
        while(!isEmptyPQ(traffic->timeEvents) && getNextPQ(traffic->timeEvents)->timeStep == traffic->masterSteps){
            event = dequeuePQ(traffic->timeEvents);
            
            if(event->type == ADD_CAR_EVENT){
                /*ADD CAR EVENT*/
                traffic->carsRemaining += getNumElements(event->carsToAdd);
                totalCars += getNumElements(event->carsToAdd);
                mergeQueues(traffic->roads[event->startRoad]->toBeAdded, event->carsToAdd);
                printf("CYCLE %d - ADD_CAR_EVENT - Cars enqueued on road from %d to %d\n",traffic->masterSteps, traffic->roads[event->startRoad]->from, traffic->roads[event->startRoad]->to);
                freeQueue(event->carsToAdd);
                free(event);
                
                
            }
            else printRoadData(traffic,event);
            
        }
        
    
        /*Loop to check if any can move to other intersections*/
        for(i=0; i < traffic->numRoads;i++){
            RoadData* tempRoad = traffic->roads[i];
            

            /*Checks if something can move to the next closest*/
            if( tempRoad->cars[tempRoad->roadLength-1] != NULL && getNextOnShortestPath(traffic->graph,tempRoad->to,tempRoad->cars[tempRoad->roadLength-1]->dest,&nextShortest) && tempRoad->isGreen){
                RoadData* moveTo = getEdgeData(traffic->graph,nextShortest,tempRoad->to);
                
                if(tempRoad->to == tempRoad->cars[tempRoad->roadLength-1]->dest){
                    
                    printf("CYCLE %d - Car successfully traveled from %d to %d in %d time steps.\n",traffic->masterSteps, tempRoad->cars[tempRoad->roadLength-1]->start, tempRoad->cars[tempRoad->roadLength-1]->dest,traffic->masterSteps - tempRoad->cars[tempRoad->roadLength-1]->stepAdded+1);
                    traffic->carsRemaining--;
                    
                    longestMove = (longestMove < traffic->masterSteps - tempRoad->cars[tempRoad->roadLength-1]->stepAdded+1) ? traffic->masterSteps - tempRoad->cars[tempRoad->roadLength-1]->stepAdded+1 : longestMove;
                    avgSteps += traffic->masterSteps - tempRoad->cars[tempRoad->roadLength-1]->stepAdded+1;
                    free(tempRoad->cars[tempRoad->roadLength-1]);
                    tempRoad->cars[tempRoad->roadLength-1] = NULL;
                    hasAnythingMoved = 0;
                }
                
                else if(moveTo->cars[0] == NULL){
                    
                    moveTo->cars[0] = tempRoad->cars[tempRoad->roadLength-1];
                    moveTo->cars[0]->hasMoved = traffic->masterSteps;
                    tempRoad->cars[tempRoad->roadLength-1] = NULL;
                    hasAnythingMoved = 0;
                    
                }
            }
        }
        
        
        /*Loop to check if any of the back things can be added to the queues*/
        for(i = 0; i < traffic->numRoads;i++){
            RoadData* tempRoad = traffic->roads[i];
            /*Checks to see if we can move something into the road*/
            
            if(!isEmpty(tempRoad->toBeAdded) && tempRoad->cars[0] == NULL){

                tempRoad->cars[0] = dequeue(tempRoad->toBeAdded);

                tempRoad->cars[0]->hasMoved = traffic->masterSteps;
                
                
                hasAnythingMoved = 0;
            }
        }
        
        /*Loop to check middle*/
        for(i=0; i < traffic->numRoads;i++){
            RoadData* tempRoad = traffic->roads[i];
            for(j = tempRoad->roadLength-2; j >=0;j--){
                
                if(tempRoad->cars[j] != NULL && tempRoad->cars[j+1] == NULL && tempRoad->cars[j]->hasMoved != traffic->masterSteps){
                    
                    tempRoad->cars[j+1] = tempRoad->cars[j];
                    tempRoad->cars[j+1]->hasMoved = traffic->masterSteps;
                    tempRoad->cars[j] = NULL;
                    hasAnythingMoved = 0;
                    j--;
                }
            }
        }
        
        
        
        /*Check for gridlock*/
        
        if(hasAnythingMoved >= traffic->gridControl && traffic->carsRemaining != 0){
            printf("CYCLE %d - Gridlock detected.\n",traffic->masterSteps+1);
            gridlock = true;
            break;
        }
        
        hasAnythingMoved++;
        traffic->masterSteps++;
    }while(!isEmptyPQ(traffic->timeEvents) || ( traffic->carsRemaining != 0 && !gridlock) );
    
    if(!gridlock){
        printf("\nAverage number of time steps to the reach their destination is %.2lf.\nMaximum number of time steps to the reach their destination is %d.\n", (double)avgSteps / totalCars,longestMove);
        
    }
    
    /*Frees all the shiii*/
    freeAllThatShii(traffic);
        
    
    
    /*Remeber to give the final data and free the shiii*/
    
    

}

/* readTrafficDataFromInputFile
 * input: char* filename of file to read
 * output: TrafficData* which stores the data of this road network
 *
 * Create a new TrafficData variable and read in the traffic data from the file whose name is stored in char* filename
 */
TrafficData* readTrafficDataFromInputFile( char* filename ){
    /* open the file */
    FILE *pFile = fopen( filename, "r" );
    int numVerts=0,income;
    int i,j,k,length, vertConnect, roadCount = 0,greenOn, greenOff,cycle, carCommands, numPrints;
    
    
    
    TrafficData* traffic = (TrafficData*)malloc(sizeof(TrafficData));
    traffic->timeEvents = createPQ();
    traffic->masterSteps = 0;
    traffic->gridControl = 0;
    traffic->carsRemaining = 0;
    
    
    fscanf(pFile,"%d %d",&numVerts,&traffic->numRoads);
    traffic->graph = createGraph(numVerts);
    traffic->roads = (RoadData**)malloc(traffic->numRoads * sizeof(RoadData));
    
    
    for(i = 0; i < numVerts;i++){
        fscanf(pFile, "%d",&income);
        for(j = 0; j < income;j++){
            fscanf(pFile,"%d %d        %d %d %d", &vertConnect,&length, &greenOn ,&greenOff ,&cycle);
            
            traffic->gridControl = (traffic->gridControl < cycle) ? cycle : traffic->gridControl;
            traffic->roads[roadCount] = (RoadData*)malloc( sizeof(RoadData));
            
            traffic->roads[roadCount]->roadLength = length;
            traffic->roads[roadCount]->roadOrder = roadCount;
            traffic->roads[roadCount]->isGreen = false;
            traffic->roads[roadCount]->greenOn = greenOn;
            traffic->roads[roadCount]->greenOff = greenOff;
            traffic->roads[roadCount]->cycleLength = cycle;
            traffic->roads[roadCount]->to = i;
            traffic->roads[roadCount]->from = vertConnect;
            traffic->roads[roadCount]->cars = (Car**)malloc(length * sizeof(Car*));
            
            for(k=0;k<length;k++)traffic->roads[roadCount]->cars[k] = NULL;
            
            setEdge(traffic->graph,vertConnect,i,length);
            
            setEdgeData(traffic->graph,i,vertConnect,traffic->roads[roadCount]);
            traffic->roads[roadCount++]->toBeAdded = createQueue();
            
        }
        
    }
    /*Read in cars */
        
    fscanf(pFile, "%d",&carCommands);
        
        
    for(i =0; i< carCommands;i++){
        Event* tempEvent = (Event*)malloc(sizeof(Event));
        tempEvent->type = ADD_CAR_EVENT;
        tempEvent->carsToAdd = createQueue();
            
        int to, from,addToEdge,dest;
            
        fscanf(pFile,"%d %d %d",&to,&from,&tempEvent->timeStep);
        tempEvent->startRoad = getEdgeData(traffic->graph,from,to)->roadOrder;
            
        fscanf(pFile,"%d",&addToEdge);
            
        for(j=0;j<addToEdge;j++){
            Car* tempVroom = (Car*)malloc(sizeof(Car));
            fscanf(pFile,"%d",&dest);
            tempVroom->start = to;
            tempVroom->dest = dest;
            /*Go back and look to see if this should be unitilazlied*/
            tempVroom->hasMoved = - 1;
            tempVroom->stepAdded = tempEvent->timeStep;
            enqueue(tempEvent->carsToAdd,tempVroom);
            }
            enqueueByPriority(traffic->timeEvents,tempEvent,tempEvent->timeStep);
        }
    
    fscanf(pFile,"%d",&numPrints);
    /*Reading in of the prints*/
    for(i=0;i<numPrints;i++){
        
        Event* tempEvent = (Event*)malloc(sizeof(Event));
        tempEvent->type = PRINT_ROADS_EVENT;
        fscanf(pFile,"%d ",&j);
        
        tempEvent->timeStep = j;
        enqueueByPriority(traffic->timeEvents,tempEvent,tempEvent->timeStep);
    }
    
    
    /* close file */
    fclose( pFile );

    return traffic; /* Optional TODO: Replace this with your TrafficData pointer */
}


void printRoadData(TrafficData* traffic,Event* event){
    int i,j;
    printf("\nCYCLE %d - PRINT_ROADS_EVENT - Current contents of the roads:\n",traffic->masterSteps);
    
    for(i=0;i<traffic->numRoads;i++){
        printf("Cars on the road from %d to %d:\n",traffic->roads[i]->from,traffic->roads[i]->to);
        
        for(j= traffic->roads[i]->roadLength - 1; j >= 0 ;j--){
            (traffic->roads[i]->cars[j] == NULL) ? printf("- ") : printf("%d ",traffic->roads[i]->cars[j]->dest);
        }
        
        (traffic->roads[i]->isGreen) ? printf("(GREEN light)\n\n") : printf("(RED light)\n\n");
    }
    
    free(event);
}

void freeData(TrafficData* traffic){
    /*Free all the data*/
    
    
    
}

/*
void updateLights(TrafficData* traffic){
    int i;
    for(i=0; i < traffic->numRoads;i++) traffic->roads[i]->isGreen = (traffic->masterSteps % traffic->roads[i]->cycleLength == traffic->roads[i]->greenOn) ? true: (traffic->masterSteps % traffic->roads[i]->cycleLength != traffic->roads[i]->greenOff) ?  : false;

    
}
*/


void updateLights(TrafficData* tData){
    int i;
    for(i = 0; i < tData->numRoads; i++){
        
        
        
        int resetTime = tData->roads[i]->cycleLength;
        int lightGreen = tData->roads[i]->greenOn;
        int lightRed = tData->roads[i]->greenOff;
        
        if(tData->masterSteps % resetTime == lightGreen){
            tData->roads[i]->isGreen = true;
        }
        else if (tData->masterSteps % resetTime == lightRed){
            tData->roads[i]->isGreen = false;
        }
    }
}


void freeAllThatShii(TrafficData* tData){

    int i,j;
    Event* tempEvent;
    RoadData* roadTemp;
    while(!isEmptyPQ(tData->timeEvents)){
            /*free all events inside*/
            tempEvent = dequeuePQ(tData->timeEvents);
            
            while(!isEmpty(tempEvent->carsToAdd)){
                /*free cars to add Q*/
                free(dequeue(tempEvent->carsToAdd));
            }
            freeQueue(tempEvent->carsToAdd);
            
            free(tempEvent);
        }
        freePQ(tData->timeEvents);
        
        
        /*freeing tData->roads*/
        for(i = tData->numRoads - 1; i >= 0; i--){
            /*free cars array*/
            roadTemp = tData->roads[i];
            for(j = roadTemp->roadLength-1; j >= 0; j--){
                /*free any cars inside*/
                if(roadTemp->cars[j] != NULL){
                    free(roadTemp->cars[j]);
                }
            }
            free(roadTemp->cars);
            
            /*free waiting car Q*/
            while(!isEmpty(roadTemp->toBeAdded)){
                /*free any cars inside*/
                free(dequeue(roadTemp->toBeAdded));
            }
            freeQueue(roadTemp->toBeAdded);
            
            free(roadTemp);
        }
        
        free(tData->roads);
        
        /*free graph*/
        freeGraph(tData->graph);
        
        /*free traffic data*/
        free(tData);
    }

