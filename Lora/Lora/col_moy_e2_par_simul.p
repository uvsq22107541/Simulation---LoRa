# Set the output to a png file
set terminal png size 1250,800
# The file we'll write to
set output 'Probabilite de collision moyenne a l etat e2 par simulation.png'
set style data histogram   
set style fill solid border -1     
set grid ytics linestyle 1      
set title ' Probabilite de collision moyenne a l etat e2 par simulation '
set ylabel " probabilite de collision %"    
set xlabel " Numero de la simulation "              
#plot the graphic
plot "collision moyenne e2 par simulation.data" using 2:xtic(1) lc 'orange'  