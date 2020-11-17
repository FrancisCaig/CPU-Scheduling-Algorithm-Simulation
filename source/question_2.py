# -*- coding: utf-8 -*-

import os
import operator
import matplotlib.pyplot as plt

# In[0]: runs simulation to get output
def run_simulaiton_varying_rr_add():

    # command './code/simulation 42 0.01 256 4 4 0.5 100'
    # os.system('./code/simulation 42 0.01 256 4 4 0.5 100 END rr_add_test_01_end_out.txt')
    # os.system('./code/simulation 42 0.01 256 4 4 0.5 100 BEGINNING rr_add_test_01_beggining_out.txt')

    # os.system('./code/simulation 42 0.01 256 8 4 0.5 100 END rr_add_test_02_end_out.txt')
    # os.system('./code/simulation 42 0.01 256 8 4 0.5 100 BEGINNING rr_add_test_02_beggining_out.txt')

    # os.system('./code/simulation 42 0.01 256 16 4 0.5 100 END rr_add_test_03_end_out.txt')
    # os.system('./code/simulation 42 0.01 256 16 4 0.5 100 BEGINNING rr_add_test_03_beggining_out.txt')

    os.system('./code/simulation 42 0.01 256 32 4 0.5 100 END rr_add_test_04_end_out.txt')
    os.system('./code/simulation 42 0.01 256 32 4 0.5 100 BEGINNING rr_add_test_04_beggining_out.txt')

# In[0]: main function
def main():

    run_simulaiton_varying_rr_add()

if  __name__ == '__main__': main()
