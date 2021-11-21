package util;
import java.util.HashMap;
import java.util.Set;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
public class CharOccur{

    private HashMap<Character, Integer> occurances;
    private String chars;

    public CharOccur(String s){
        chars = s;
        occurances = new HashMap<Character, Integer>();
    }

    public CharOccur(File f){
        try(BufferedReader r = new BufferedReader(new FileReader(f))){
            chars = r.readLine();
            while(r.readLine() != null)
                chars += r.readLine();
            occurances = new HashMap<Character, Integer>();
        }catch(java.io.IOException e){
            System.out.println("no such file");
            e.printStackTrace(); 
            System.exit(0);
        }
    }

    public void countOccur(){
        for(int i = 0; i < chars.length(); i++){
            if(!occurances.containsKey(chars.charAt(i)))
                occurances.put(chars.charAt(i), 1);
            else
                occurances.put(chars.charAt(i), 
                        occurances.get(chars.charAt(i))+1);
        }
    }

    public void printOccur(){
        Set<Character> ocSet = occurances.keySet();
        for(Character c : ocSet){
            System.out.print(c + " : ");
            for(int i = 0; i < occurances.get(c); i++)
                System.out.print("▒");
            System.out.println(" (" + occurances.get(c) + ")");
        }
            
    }

    public static void main(String[] args){
        File f = new File("test.txt");
        CharOccur co = new CharOccur(f);
        co.countOccur();
        co.printOccur();
    }
}
