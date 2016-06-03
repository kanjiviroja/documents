#!/usr/bin/env python

prices = {'apple': 0.40, 'banana': 0.50}
my_purchase = {
    'apple': 5,
    'banana': 7}
grocery_bill = sum(prices[fruit] * my_purchase[fruit]
                   for fruit in my_purchase)
print 'I owe the grocer $%.2f' % grocery_bill