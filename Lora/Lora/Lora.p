# Set the output to a png file
set terminal png size 1250,800
# The file we'll write to
set output 'Probabilite de collision par etats.png'
# The graphic title
set title ' Probabilite de collision par etats en fonction du temps '
set ylabel " probabilite %"    
set xlabel " Time "             
#plot the graphic
plot "Lora.data" using 1:2 title "etat 1" with lines,\
"Lora.data" using 1:2 title "etat 1" with lines lw 2,\
"Lora.data" using 1:3 title "etat 2" with lines lw 2,\
"Lora.data" using 1:4 title "etat 3" with lines lw 2,\
"Lora.data" using 1:5 title "etat 4" with lines lw 2,\
"Lora.data" using 1:6 title "etat 5" with lines lw 2,\
"Lora.data" using 1:7 title "etat 6" with lines lw 2,\
"Lora.data" using 1:8 title "etat 7" with lines lw 2