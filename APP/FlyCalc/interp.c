#include "interp.h"
#include "math.h"


//线性插值
double inter_linear(double x0,double x1,double y0,double y1,double x)    
{    
    double a0,a1,y;    
    a0=(x-x1)/(x0-x1);    
    a1=(x-x0)/(x1-x0);    
    y=a0*y0+a1*y1;    
   
    return (y);    
} 

//一维插值
double interp1(double x[],double y[],int n,double pp)   
{   
    double z;   
    int i,tmp;   
 
    for(i = 0;i<n;i++)   
    {   
        if((pp>=x[i])&&(pp<x[i+1]))   
        {   
            tmp = i;   
            break;   
        }   
    }   
 
    if(i == n)   
    {   
        z = y[i-1];   
        return z;   
    }   
    z = inter_linear(x[tmp],x[tmp+1],y[tmp],y[tmp+1],pp);   
   
    return z;   
}   


//二维插值
double interp2(double x[], double y[], double **z, int m, int n,   
                double a, double b)   
{   
    int i,j,tmpi,tmpj;   
    double w,w1,w2;   
    tmpi = 0;   
    tmpj = 0;   
    w = 0.0;   
    /**********************/   
    for(i=0; i<m; i++)                     
    {   
        if( (a>=x[i])&&(a<x[i+1]) )        
        {   
            tmpi = i;   
            break;   
        }   
    }   
    for(j=0; j<n; j++)              
    {   
        if( (b>=y[j])&&(b<y[j+1]) )          
        {   
            tmpj = j;   
            break;   
        }   
    }   
    /***************************/   
    if(x[tmpi] == a)   
    {   
        w1 = z[tmpi][tmpj];   
        w2 = z[tmpi][tmpj+1];   
   
        /***************/   
        if(y[tmpj] == b)   
        {   
            //  
            w = w1;   
        }   
        else   
        {   
            //  
            w = inter_linear(y[tmpj],y[tmpj+1],w1,w2,b);   
        }   
        /***************************/   
    }   
    else   
    {   
        //   
        w1 = inter_linear(x[tmpi],x[tmpi+1],z[tmpi][tmpj],z[tmpi+1][tmpj],a);   
        w2 = inter_linear(x[tmpi],x[tmpi+1],z[tmpi][tmpj+1],z[tmpi+1][tmpj+1],a);   
   
        /***************/   
        if(y[tmpj] == b)   
        {   
            //  
            w = w1;   
        }   
        else   
        {   
            w = inter_linear(y[tmpj],y[tmpj+1],w1,w2,b);   
        }   
        /****************************/   
    }   
   
    return w;   
}   
