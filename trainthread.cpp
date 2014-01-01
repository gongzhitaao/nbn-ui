#include "trainthread.h"

#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "nbn.h"

TrainThread::TrainThread(NBN *nbn, int num_run, int max_iteration, double max_error,
                         QString data_file, QObject *parent) :
    nbn_(nbn),
    num_run_(num_run),
    max_iteration_(max_iteration),
    max_error_(max_error),
    data_file_(data_file)
{
}

void TrainThread::run()
{
    int num_input = nbn_->get_num_input();
    int num_output = nbn_->get_num_output();
    std::ifstream fin(data_file_.toStdString());
    std::vector<double> inputs, desired_outputs;
    std::string temp;
    while (getline(fin, temp)) {
        std::stringstream buffer(temp);
        std::vector<double> pattern(std::istream_iterator<double>(buffer),
                                    std::istream_iterator<double>());
//        inputs.insert(inputs.end(), pattern.begin(), pattern.begin() + num_input);
//        desired_outputs.insert(desired_outputs.end(), pattern.begin() + num_output,
//                               pattern.end());
    }
}
