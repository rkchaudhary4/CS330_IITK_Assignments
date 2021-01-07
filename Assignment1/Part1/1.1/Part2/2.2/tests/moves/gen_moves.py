import random

for i in range(1, 100):
    f = open('moves{}.txt'.format(i), 'w')
    for j in range(1000):
        f.write('{}\n'.format(random.randint(0, 2)));
    f.close();
