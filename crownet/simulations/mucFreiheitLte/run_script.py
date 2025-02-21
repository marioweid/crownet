#!/usr/bin/env python3
import sys, os

from datetime import datetime
from roveranalyzer.simulators.crownet.runner import BaseRunner


class SimulationRun(BaseRunner):
    def __init__(self, working_dir, args=None):
        super().__init__(working_dir, args)
        
    # todo: add pre and post processing to apply for each simulation run.
    # Post processing might inlcude result striping or aggreagtion. 
    # add as many methods as needed and add the process_as annotation.
    # Use `prio` to ensure exeuction order (Bigger number higher prio)
    
    # @process_as({"prio": 20, "type": "post"})
    # def foo(self):
        # pass
    
    # @process_as({"prio": 10, "type": "pre"})
    # def bar(self):
        # pass


if __name__ == "__main__":

    # todo: add settings manualy
    settings = [
    ]

    if len(sys.argv) == 1:
        # default behavior of script
        runner = SimulationRun(os.getcwd(), settings)
    else:
        # use arguments from command line
        runner = SimulationRun(os.path.dirname(os.path.abspath(__file__)))

    runner.run()
