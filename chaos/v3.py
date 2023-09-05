# Experiment in creating random BASICally programs.
# Used to help determine if the idea of making Randomize in BASICally
# do something useful, and possibly as a path to making a native module
# that creates these structures and can execute them more efficiently.

# V2 removes the dependancy tree being limited to variables that are already
# known to be set.

# TODO: Prevent the program from just completely ignoring an attached INx
# variable.

import random

# operand_names is a list of any size.
def SimpleExpr(result_name, operand_names):
    print(f'{result_name} = {operand_names[0]}')
    for i in range(1, len(operand_names)):
        operand = random.choice(['+', '-', '*', '/'])
        print(f'    {operand} {operand_names[i]}')

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
    print(f'{result_name} = {name1} * {name2}')

def MultConst(result_name, name1):
    # Picking a value to multiply by.
    const = random.uniform(0.4, 1.1)
    print(f'{result_name} = {name1} * {const:.3f}')

def Envelope(result_name, name1):
    print(f'{result_name} = abs({name1})')

def Sin(result_name, name1):
    print(f'{result_name} = sin({name1} / 5 * 3.14) * 5')

def Floor(result_name, name1):
    # Sets all samples below 0.0 to 0.0.
    print(f'IF {name1} < 0 THEN')
    print(f'  {result_name} = 0')
    print(f'ELSE')
    print(f'  {result_name} = {name1}')
    print(f'END IF')

def Switch(result_name, name1, name2, name3):
    tick_count = random.randint(9000, 20000)
    print(f'{result_name}_tick = mod({result_name}_tick + 1, {tick_count})')
    print(f'IF {result_name}_tick == 1 THEN')
    print(f'  IF {name1} > 0 THEN {result_name}_choice = 0 ELSE {result_name}_choice = 1 END IF')
    print(f'END IF')
    print(f'IF {result_name}_choice > 0 THEN {result_name} = {name2} ELSE {result_name} = {name3} END IF')

funcs = [Add, AddConst, Subtract, SubtractConst, Mult, MultConst, Sin, Envelope, Floor, Switch]
func_args = [2, 1, 2, 1, 2, 1, 1, 1, 1, 3]

def Assign(result, inputs, preferred_inputs = []):
    if random.random() < 0.5:
        which = random.randrange(0, len(funcs))
        pref_args = []
        if len(preferred_inputs):
            pref_args = random.sample(preferred_inputs,
                                      min(func_args[which], len(preferred_inputs)))
            if len(pref_args) < func_args[which]:
                more_args = random.sample(inputs, func_args[which] - len(pref_args))
                these_args = [x for x in pref_args + more_args]
            else:
                these_args = [x for x in pref_args]
        else:
            these_args = random.sample(inputs, func_args[which])
        if func_args[which] == 1:
            funcs[which](result, these_args[0])
        elif func_args[which] == 2:
            funcs[which](result, these_args[0], these_args[1])
        elif func_args[which] == 3:
            funcs[which](result, these_args[0], these_args[1], these_args[2])
        return these_args
    else:
      if len(preferred_inputs):
          SimpleExpr(result, preferred_inputs)
          return preferred_inputs
      else:
          num_operands = random.randrange(2, len(inputs))
          these_args = random.sample(inputs, num_operands)
          SimpleExpr(result, these_args)
          return these_args

def Pulse():
    for name in ['pulse1', 'pulse2']:
        print(f'\nALSO')
        wait = random.randint(1, 1500)
        wait2 = random.randint(1, 1500)
        print(f'  WAIT {wait}')
        print(f'  FOR {name} = 0 TO 2 STEP 0.01')
        print(f'  NEXT')
        print(f'  WAIT {wait2}')
        print(f'  FOR {name} = 2 TO 0 STEP -0.01')
        print(f'  NEXT')
        print(f'END ALSO')

    print(f'\nALSO')
    print('  state_wait = random(100, 1000)')
    print('  WAIT state_wait')
    print('  state = floor(random(0, 3))')
    print(f'END ALSO')
    print(f'\nWHEN start()')
    print(f'  CLEAR ALL')
    print(f'END WHEN')

# static lists so I can do special things when needed.
inputs = ['in1', 'in2', 'in3']
outputs = ['out1', 'out2', 'out3']

# assigned is list of variables that we are confident have been set.
# we add to as assignements are made, so future lines can use them.
assigned = ['in1', 'in2', 'in3', 'pulse1', 'pulse2', 'a', 'b', 'c', 'out1', 'out2', 'out3']

# needed is the list of variables that must be assigned by program end.
# We remove from it once it's been assigned.
needed = ['out1', 'out2', 'out3']

# must_use are the inputs that must be read. Without this, often inputs
# are just ignored, which is unsatisfying.
must_use = ['in1', 'in2', 'in3']

# Let's add some intermediate values to spice things up.
# We mention some twice to compute them twice.
starters = ['a', 'b', 'c']
assigned = ['in1', 'in2', 'in3', 'pulse1', 'pulse2', 'a', 'b', 'c']

while len(starters):
    result = starters.pop(0)
    used = Assign(result, assigned)
    must_use = [x for x in must_use if x not in used]

extra_pool = ['ex1', 'ex2', 'ex3']
assigned = ['a', 'b', 'c']

while len(must_use):
    result = extra_pool.pop(0)
    assigned.append(result)
    used = Assign(result, assigned, must_use)
    must_use = [x for x in must_use if x not in used]

for state in ['0', '1', '2']:
  print(f'\nIF state == {state} THEN')
  #  second_layer = ['d', 'e', 'f']
  second_layer = ['d']
  while len(second_layer):
    result = second_layer.pop(0)
    Assign(result, assigned)
  print(f'END IF')

assigned = ['a', 'b', 'c', 'd']
print()
while len(needed):
    result = needed.pop(0)
    SimpleExpr(result, assigned)
    Fold(result)

Pulse()
