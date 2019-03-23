#!/bin/env python3

from sys import argv
from sys import exit
from collections import Counter

"""
Naive parser and pretty printer for the MISRA reports by cppcheck
"""

class Summarizer(object):
    def __init__(self, report_name):
        with open(report_name, 'r') as f:
            self.file_lines = f.readlines()
        f.close()
        
        self.red = "\033[91m"
        self.yellow = "\033[93m"
        self.green = "\033[92m"
        self.bold = "\033[1m"
        self.end = "\033[0m"


    def analyze(self):
        """
        A really dumb parser for the pre-processed report generated by cppcheck
        """
    
        errors_map = {} # dictionary containing filenames, rule violations and line where the violation occurred
    
        lines_seen = set() # contains the unique lines from the file
    
        if len(self.file_lines) == 0:
            print(bold + green + "Static analysis for MISRA compliance complete. No errors found." + end)
            return 0
        else:
            for line in self.file_lines:  # remove duplicate lines
                if line not in lines_seen:
                    lines_seen.add(line)
            
                    line_contents = line.split(':')
                    file_name = line_contents[0] # first part is the filename (index 0)
                    error = (line_contents[1], line_contents[2].strip('\n')) # index 1 is the line number, index 2 is the number of violated rule

                    if file_name not in errors_map.keys():
                        errors_map[file_name] = list()  # create a new list for the new filename and append the tuple in it
                        errors_map[file_name].append(error)
                    else:
                        errors_map[file_name].append(error) # do not append if it already exists
            
            return errors_map # return the completed error dictionary


    def pretty_print(self, errors):
        """
        Pretty-prints the contents of the error dictionary with colors and stuff
        """

        print(self.bold + self.red + "=================================================\n" + self.end)
        print(self.bold + self.red + "       Static analysis results: Error Summary        \n" + self.end)
        for key in errors:
            for error in errors[key]:
                name_string = f"{self.bold}{self.red}File {self.yellow}{key}{self.red}"
                rule_violated_string = f"violates rule {self.yellow}#{error[1]}{self.red} of the MISRA C 2012 standard"
                line_number_string = f"at line {self.yellow}{error[0]}{self.end}"

                print(f"{name_string.ljust(75)} {rule_violated_string} {line_number_string}")

        print("")
        print("")
        print(self.bold + self.red +"=================================================" + self.end)


if __name__ == "__main__":
    s = Summarizer(argv[1])
    errors = s.analyze()
    if isinstance(errors, dict):
        s.pretty_print(errors)
        exit(127)
    elif isinstance(errors, int) and errors == 0:
        exit(0)
