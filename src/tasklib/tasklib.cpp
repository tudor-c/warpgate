#include <string>
#include <format>
#include <vector>
#include <sstream>
#include <unistd.h>

extern "C" std::string even(const std::vector<std::string>& args) {
    sleep(15);
    std::stringstream res;
    int val;
    std::stringstream in(args[0]);
    while (in >> val) {
        if (val % 2 == 0) {
            res << val << ' ';
        }
    }
    return res.str();
}

extern "C" std::string odd(std::vector<std::string>& args) {
    sleep(10);
    std::stringstream res;
    int val;
    std::stringstream in(args[0]);
    while (in >> val) {
        if (val % 2 == 1) {
            res << val << ' ';
        }
    }
    return res.str();
}

extern "C" std::string primes(std::vector<std::string>& args) {
    sleep(5);
    std::stringstream res;
    int val;
    std::stringstream in(args[0]);
    while (in >> val) {
        bool prime = true;
        for (int i = 2; i < val; i++) {
            if (val % i == 0) {
                prime = false;
            }
        }
        if (prime) {
            res << val << ' ';
        }
    }
    return res.str();
}

extern "C" std::string nth_fibo(const std::vector<std::string>& args) {
    sleep(18);
    const int n = std::stoi(args[0]);
    int aux0 = 0;
    int aux1 = 1;
    int aux2 = 0;
    for (int i = 2; i <= n; i++){
        aux0 = aux1 + aux2;
        aux2 = aux1;
        aux1 = aux0;
    }
    return std::to_string(aux0);
}

extern "C" std::string n_num(const std::vector<std::string>& args) {
    sleep(3);
    return args[0];
}

extern "C" std::string cartesian(const std::vector<std::string>& args) {
    sleep(7);
    std::stringstream outStream;
    std::vector<std::string> res;
    std::vector<int> arr1;
    std::vector<int> arr2;

    std::stringstream in1(args[0]);
    std::stringstream in2(args[1]);
    std::string aux;

    while (in1 >> aux) {
        arr1.push_back(std::stoi(aux));
    }
    while (in2 >> aux) {
        arr2.push_back(std::stoi(aux));
    }

    for (int i = 0; i < arr1.size(); i++) {
        for (int j = 0; j < arr2.size(); j++) {
            res.push_back(std::to_string(arr1[i] * arr2[j]));
        }
    }

    for (auto& s : res) {
        outStream << s << ' ';
    }
    return outStream.str();
}

extern "C" std::string keep10(const std::vector<std::string>& args) {
    sleep(10);
    std::stringstream res;
    std::string val;
    std::stringstream in(args[0]);
    for (int i = 0; i < 10 && in >> val; i++) {
        res << val << ' ';
    }
    return res.str();
}

extern "C" std::string maximum(const std::vector<std::string>& args) {
    sleep(10);
    if (std::stoi(args[0]) > std::stoi(args[1])) {
        return args[1];
    }
    return args[0];
}

extern "C" std::string square(const std::vector<std::string>& args) {
    sleep(10);
    int val = std::stoi(args[0]);
    return std::to_string(val * val);
}
