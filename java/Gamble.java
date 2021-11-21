package gamble;

import java.util.Random;
import java.util.Scanner;

public class Gamble{
    private static final int ITERATIONS = 625000;

    public static void main(String[] args){
        Scanner s = new Scanner(System.in);
        System.out.print("Wie viele Felder?\n> ");
        int l = s.nextInt();
        System.out.print("Wie viele Moeglichkeiten?\n> ");
        int cl = s.nextInt();
        int[] a = play(l, cl);
        for(int i = 0; i < a.length; i++){
            System.out.println(a[i] + " mal " + i + " Treffer (" + 
                    a[i]/((double)ITERATIONS) * 100 + "%)");
        }
    }
    
    public static int[] play(int length, int choiceLength){
        int[] results = new int[length + 1];
        int correctChoices;
        Random r = new Random();
        for(int i = 0; i < ITERATIONS; i++){
            correctChoices = 0;
            for(int j = 0; j < length; j++){
                int choice = r.nextInt(choiceLength);
                int result = r.nextInt(choiceLength);
                if(choice == result)
                    correctChoices++;
            }
            results[correctChoices]++;
        }
        return results;
    }
}
