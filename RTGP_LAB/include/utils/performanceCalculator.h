#include <deque>
#include <iostream>

class PerformanceCalculator
{
private:
    unsigned int windowSize;
    unsigned int overlap;
    unsigned int iter;
    std::deque<double> measurements; 
    static double CalculateMeanMeasurements(std::deque<double> queue){
        double mean = 0.0f;
        for(size_t i = 0; i < queue.size(); i++){
            mean += queue[i];
        }
        mean /= queue.size();
        return mean;
    };
public:
    double framerate;
    PerformanceCalculator(unsigned int windowDim, unsigned int printAfterNSteps){
        this->windowSize = windowDim;
        this->overlap = printAfterNSteps;
        this->iter = 0;
    };
    ~PerformanceCalculator(){
        delete &this->measurements;
    };

    void Step(double deltaTime){
        this->measurements.push_back(deltaTime);

        unsigned int vectorDim = this->measurements.size();
        if(vectorDim >= windowSize){
            this->measurements.pop_front();
        }

        this->iter++;

        if(this->iter >= overlap){
            this->framerate = 1 / CalculateMeanMeasurements(this->measurements);
            this->iter = 0;
        }
    }

    void LogStatus(){
        std::cout << "Log of the Performance Calculator" << std::endl;
        std::cout << "Size: " << this->measurements.size() << std::endl;
        std::cout << "Framerate: " << this->framerate << std::endl;
        for(size_t i = 0; i < this->measurements.size(); i++){
            std::cout << "Elm " << i << " :" << this->measurements[i] << std::endl;
        }
        std::cout << "---END Performance Calculator LOG STATUS ---" << std::endl;
        std::cout << std::endl;
    }
};
