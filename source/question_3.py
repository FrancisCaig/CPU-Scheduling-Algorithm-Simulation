# -*- coding: utf-8 -*-

import os
import operator
import matplotlib.pyplot as plt

# In[0]: runs simulation to get output
def run_simulaiton_varying_a():

    # command './code/simulation 2 0.01 256 16 4 <0.5> 128 sim_out.txt'
    os.system('./code/simulation 42 0.01 256 16 4 0.1 128 alpha_test_01_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.15 128 alpha_test_02_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.2 128 alpha_test_03_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.25 128 alpha_test_04_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.3 128 alpha_test_05_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.35 128 alpha_test_06_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.4 128 alpha_test_07_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.45 128 alpha_test_08_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.5 128 alpha_test_09_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.55 128 alpha_test_10_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.6 128 alpha_test_11_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.65 128 alpha_test_12_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.7 128 alpha_test_13_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.75 128 alpha_test_14_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.8 128 alpha_test_15_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.85 128 alpha_test_16_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.9 128 alpha_test_17_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 0.95 128 alpha_test_18_out.txt')
    os.system('./code/simulation 42 0.01 256 16 4 1.0 128 alpha_test_19_out.txt')

# In[0]: parse simulation output
def parse_simoulation_output(output_path):

    # store results
    alpha_list = []
    sjf_wait_list = []
    sjf_turn_list = []
    sjf_context_list = []

    srt_wait_list = []
    srt_turn_list = []
    srt_context_list = []

    # list files of this directory
    infiles = sorted(os.listdir(output_path))
    alpha = 0.10

    # loop over input files
    for file in infiles:

        # skip non txt files
        if not file.endswith('.txt'):
            continue

        # print("\tParsing file: " + file)

        algorithm = 'none'
        lines_to_read = 0
        with open(os.path.join(output_path, file), 'r') as sim_output:
            alpha_list.append(round(alpha, 2))

            for line in sim_output:
                line = line.strip()

                if line == 'Algorithm SJF':
                    lines_to_read = 5
                    algorithm = 'sjf'

                elif line == 'Algorithm SRT':
                    lines_to_read = 5
                    algorithm = 'srt'

                elif lines_to_read > 0:

                    # parse line
                    if lines_to_read == 4:
                        data = line.replace('-- average wait time: ', '')
                        data = data.replace(' ms', '')
                        data = float(data)

                        if algorithm == 'sjf':
                            sjf_wait_list.append(data)

                        elif algorithm == 'srt':
                            srt_wait_list.append(data)

                    elif lines_to_read == 3:
                        data = line.replace('-- average turnaround time: ', '')
                        data = data.replace(' ms', '')
                        data = float(data)

                        if algorithm == 'sjf':
                            sjf_turn_list.append(data)

                        elif algorithm == 'srt':
                            srt_turn_list.append(data)

                    elif lines_to_read == 2:
                        data = line.replace('-- total number of context switches: ', '')
                        data = data.replace(' ms', '')
                        data = int(data)

                        if algorithm == 'sjf':
                            sjf_context_list.append(data)

                        elif algorithm == 'srt':
                            srt_context_list.append(data)

                    lines_to_read -= 1

                    if lines_to_read == 0:
                        algorithm = 'none'

                else:
                    continue

        # alpha step between runs
        alpha += 0.05

    return alpha_list, sjf_wait_list, sjf_turn_list, sjf_context_list, srt_wait_list, srt_turn_list, srt_context_list

# In[0]: plots simulation results
def plot_simulation_results(alpha_list, wait_list, turn_lis, context_list = []):

    # get minimum value index
    wait_min_index, wait_min_value = min(enumerate(wait_list), key = operator.itemgetter(1))
    turn_min_index, turn_min_value = min(enumerate(turn_lis), key = operator.itemgetter(1))

    x_ticks = []
    for alpha in alpha_list:
        if str(alpha)[-1] != '5' or alpha == 0.5:
            x_ticks.append(alpha)

    fig = plt.figure(figsize = (10, 5))
    ax = fig.add_subplot(111)

    ax.plot(alpha_list, wait_list, '-gD', label = 'Avg Wait Time (ms)')
    ax.axvline(x = alpha_list[wait_min_index], color = 'g', linestyle = '--', label = 'Min Wait Time')

    ax.plot(alpha_list, turn_lis, '-rH', label = 'Avg Turnaround Time (ms)')
    # ax.axvline(x = alpha_list[turn_min_index], color = 'r', linestyle = '--', label = 'Min Turnaround Time')

    # also plot context switches
    if len(context_list) > 0:
        context_min_index, context_min_value = min(enumerate(context_list), key = operator.itemgetter(1))

        ax.plot(alpha_list, context_list, '-bp', label = 'Total Context Switches (count)')
        ax.axvline(x = alpha_list[context_min_index], color = 'b', linestyle = '--', label = 'Min Context Switches')

    ax.set_title('Algorithm statistics vs. Alpha')
    ax.legend(loc = 'best')
    ax.set_xlabel('Alpha')
    plt.xticks(x_ticks)

    plt.show()

# In[0]: main function
def main():

    # run_simulaiton_varying_a()
    alpha_list, sjf_wait_list, sjf_turn_list, sjf_context_list, srt_wait_list, srt_turn_list, srt_context_list = parse_simoulation_output('./')

    # plot SJF
    plot_simulation_results(alpha_list, sjf_wait_list, sjf_turn_list)
    plot_simulation_results(alpha_list, srt_wait_list, srt_turn_list, srt_context_list)

if  __name__ == '__main__': main()
