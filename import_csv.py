import csv

with open('customer.csv', encoding='utf-8') as f:
    reader = csv.reader(f)
    print("header:")
    header = next(reader)
    print(header)
    print(type(header))
    listv = header[0].split("|")
    print(listv[0])
    print(type(listv[0]))
    print("reader:")
    for row in reader:
        listr = row[0].split("|")
        print(listr[0])
        a = int(listr[0]) + 5
        #print(float(d))
        print(a)
        print(row)
