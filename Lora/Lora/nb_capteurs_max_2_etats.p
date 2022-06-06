# Set the output to a png file
set terminal png size 1250,800
# The file we'll write to
set output 'nombre de capteurs max pour que un message soit envoyé en 2 tentatives maximum.png'
set style data histogram   
set style fill solid border -1     
set grid ytics linestyle 1      
set title ' nombre de capteurs max pour que un message soit envoyé en 2 tentatives maximum '
set ylabel " Nombre de capteur K"    
set xlabel " Numero de la simulation "  
set yrange[ 0 to 5 ]            
#plot the graphic
plot "nombre de capteurs max pour 2etats.data" using 2:xtic(1) lc 'red'  