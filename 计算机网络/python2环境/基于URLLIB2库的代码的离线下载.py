from urllib2 import *
f = urlopen("http://host.docker.internal:8080")
g=f.read()
print g
outfile = open("output.txt","w")
outfile.write(g)
outfile.close()
