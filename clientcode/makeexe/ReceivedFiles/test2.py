import math
from sympy import symbols, integrate, Eq, solve  
from scipy.integrate import quad
import numpy as np
m_list=[]
m=np.array([1,2])
m_list.append(m)
m_list.append(m)
m_list=np.array(m_list)
print(m_list)
## 定义变量  
#x,a1,a2,a3= symbols(['x','a1','a2','a3'])  
  
## 定义被积函数  
#integrand = (1+a1+2*a2*x+2*a3*x**2)**(0.5)
  
## 使用 integrate 函数求解不定积分  
#result = integrate(integrand, x)  
  
#print("不定积分结果:", result)

# 定义被积函数  
#def integrand(x):  
#    return math.sqrt(1+1+x+x**2)  # 这里是 x^2 作为例子  
  
## 使用 quad 函数求解从 0 到 1 的定积分  
#result, error = quad(integrand, 4, 5)  
  
#print("积分结果:", result)  
#print("估计误差:", error)
#print(math.log(2.718))
# coeffs=[1,1,-2,6,-3,4]
# roots=np.roots(coeffs)
# for r in roots:
#     if (r.imag==0):
#         print(r.real)
# print(roots)
# def fun(m):
#     m[0]=3
#     return 0

# fun(coeffs)
# print(coeffs)