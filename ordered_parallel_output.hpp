#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
// n+1 threads, where n is the number of values provided in the args vector
namespace ordered_parallel{
    // template to allow all function/argument types, takes in a function, a vector of arguments of generic type, and an outstream to write the output to
    template <typename func, typename... args>
    void ordered_parallel_output(std::ostream& out, func input_function, const std::vector<std::tuple<args...>>& input_args){
        using result = std::invoke_result_t<func, args...>;
        int n_shrapnel = input_args.size();
        // make a map where int is the value of the order of the input, and result is the output written
        std::map<int, result> completed_fragments;
        std::mutex mtx;
        std::condition_variable cond;
        int next_block = 0;
        // lambda worker which passes global vars by reference to compute one function call
        auto run_function = [&](int id, std::tuple<args...> curr_args){
            result one_output = std::apply(input_function, curr_args);
            // mutex lock the thread to make sure only one write to completed_parts happens at once
            {
                std::lock_guard<std::mutex> lock(mtx);
                // use std::move so that we don't destroy our performance by copying the value (wow that's cool)
                completed_fragments[id] = std::move(one_output);
            } 
            // update that one result is done!
            cond.notify_one();
        };
        // function to gather and write the output to the final stream
        auto write_output = [&](){
            while (next_block < n_shrapnel){
                std::unique_lock<std::mutex> lock(mtx);
                // suspend this thread until the actual fragment we're looking for next in line is ready
                cond.wait(lock, [&] {return completed_fragments.count(next_block);});
                // when we're done with the next block, write it to the output
                out << completed_fragments[next_block];
                // free the memory associated with that block, we don't need it anymore
                completed_fragments.erase(next_block);
                // prep for the next block
                next_block++;
            }
        };
        // set up a vector of threads to act as function runners
        std::vector<std::thread> threads;
        for (int i = 0; i < n_shrapnel; i++){
            // spawn a thread with the function(id, input_arg)
            threads.emplace_back(run_function, i, input_args[i]);
        }
        std::thread writer_thread(write_output);
        // wait for everything to finish before returning
        for (auto& t : threads) t.join();
        writer_thread.join();
    }
}