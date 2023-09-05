# Experiment in creating random BASICally programs.
# Used to help determine if the idea of making Randomize in BASICally
# do something useful, and possibly as a path to making a native module
# that creates these structures and can execute them more efficiently.

import random

def Fold(result_name):
    print(f'IF abs({result_name}) > 5 THEN')
    print(f'  {result_name} = sign({result_name}) * 5 - mod({result_name}, 5)')
    print(f'END IF')

def Add(result_name, name1, name2):
    print(f'{result_name} = {name1} + {name2}')

def AddConst(result_name, name1):
    const = random.uniform(0.1, 1)
    print(f'{result_name} = {name1} + {const:.3f}')

def Subtract(result_name, name1, name2):
    print(f'{result_name} = {name1} - {name2}')

def SubtractConst(result_name, name1):
    const = random.uniform(0.1, 1)
    print(f'{result_name} = {name1} - {const:.3f}')

def Mult(result_name, name1, name2):
    const = random.uniform(0.2, 2)
    print(f'{result_name} = {name1} * {name2} * {const:.3f}')

def MultConst(result_name, name1):
    # Picking a value to multiply by.
    const = random.uniform(0.4, 1.1)
    print(f'{result_name} = {name1} * {const:.3f}')

def Switch(result_name, name1, name2, name3):
    tick_count = random.randint(9000, 20000)
    print(f'{result_name}_tick = mod({result_name}_tick + 1, {tick_count})')
    print(f'IF {result_name}_tick == 1 THEN')
    print(f'  IF {name1} > 0 THEN')
    print(f'    {result_name}_choice = 0')
    print(f'  ELSE')
    print(f'    {result_name}_choice = 1')
    print(f'  END IF')
    print(f'END IF')
    print(f'IF {result_name}_choice > 0 THEN\n  {result_name} = {name2}')
    print(f'ELSE\n  {result_name} = {name3}\nEND IF')

funcs = [Add, AddConst, Subtract, SubtractConst, Mult, MultConst, Switch]
func_args = [2, 1, 2, 1, 2, 1, 3]

def Assign(result, inputs):
    which = random.randint(0, len(funcs) - 1)
    these_args = random.sample(inputs, func_args[which])
    if func_args[which] == 1:
        funcs[which](result, these_args[0])
    elif func_args[which] == 2:
        funcs[which](result, these_args[0], these_args[1])
    elif func_args[which] == 3:
        funcs[which](result, these_args[0], these_args[1], these_args[2])

def Pulse():
    for name in ['pulse1', 'pulse2']:
        print(f'ALSO')
        wait = random.randint(1, 1500)
        wait2 = random.randint(1, 1500)
        print(f'  {name} = 0')
        print(f'  WAIT {wait}')
        print(f'  {name} = 1')
        print(f'  WAIT {wait2}')
        print(f'END ALSO')

# assigned is list of variables that we are confident have been set.
# we add to as assignements are made, so future lines can use them.
assigned = ['in1', 'in2', 'in3', 'pulse1', 'pulse2']
# needed is the list of variables that must be assigned by program end.
# We remove from it once it's been assigned.
needed = ['out1', 'out2', 'out3']

# Let's add some intermediate values to spice things up.
starters = ['a', 'b', 'c', 'd', 'e', 'f']
second_layer = []

while len(starters):
    result = starters.pop(0)
    Assign(result, assigned)
    assigned.append(result)
    second_layer.append(result)

third_layer = second_layer.copy()
# recompute everything in second_layer
while len(third_layer):
    result = third_layer.pop(0)
    Assign(result, second_layer)

while len(needed):
    result = needed.pop(0)
    Assign(result, second_layer)
    Fold(result)
    assigned.append(result)

Pulse()
