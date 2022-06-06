# Set the output to a png file
set terminal png size 1250,800
# The file we'll write to
set output 'Probabilite de collision moyenne en fonction du nombre de capteurs.png'
# The graphic title
set title ' Proba de collision moyennes en fonction du nombre de capteur '
set ylabel " probabilite moyenne %"    
set xlabel " Nombre de capteurs "
set palette defined ( 0 'green', 40 'green', 50 'orange', 70 'orange', 80 'red', 100 'red' )              
#plot the graphic
plot "Collisions moyenne en fct du nb capteurs.data" using 1:2:2 title "proba de collisions moyenne en fct du nombre de capteurs" with lines lw 4 lc palette 