clear all; close all; clc;

force = load('../results_single_edge_notched_tension_test/force.dat');
displacements = load('../results_single_edge_notched_tension_test/displacements.dat');

plot(displacements(:,2), force(:,2));