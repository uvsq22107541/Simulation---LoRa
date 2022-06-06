# Set the output to a png file
set terminal png size 1250,800
# The file we'll write to
set output 'Temps d emission e1.png'
set style data histogram   
set style fill solid border -1     
set grid ytics linestyle 1      
set title ' Temps d emission e1 en fonction du temps '
set ylabel " Temps d'emission "    
set xlabel " temps de la simulation "              
#plot the graphic
plot "temps d emission e1.data" using 2:xtic(1) title "temps d'emission e1 " lc 'green'   