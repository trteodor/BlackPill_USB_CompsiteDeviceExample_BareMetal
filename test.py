import math

print ('Dla równania kwadratowego ax2+bx+c=0')
a=int(input('podaj wartość parametru a: '))
b=int(input('podaj wartość parametru b: '))
c=int(input('podaj wartość parametru c: '))
delta = (b**2)-(4*a*c)
print(delta)
if delta > 0:
    x1 = (-b-math.sqrt(delta))/(2*a)
    x2 = (-b+math.sqrt(delta))/(2*a)
    print('x1 = ', x1, "x2= ", x2)
elif delta == 0:
    x0 = -b/(2*a)
    print ('x0 = '), x0
else:
    print ('brak rozwiązań')