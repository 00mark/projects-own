package crack;
import java.util.Scanner;
import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
public class CaesarCrack{

    public static String shift(String cipher){
        String shifted = "";
        for(int i = 0; i < cipher.length(); i++){
            if((cipher.charAt(i) > 64 && cipher.charAt(i) < 91) || 
                    (cipher.charAt(i) > 96 && cipher.charAt(i) < 123)){
                char c = (char)(cipher.charAt(i) + 1);
                shifted += c > 122 ? 'a' : c > 90 && c < 96 ? 'A' : c;
            }
            else
                shifted += cipher.charAt(i);
        }
        return shifted;
    }

    public static String decipher(String cipher){
        cipher = cipher.toLowerCase();
        cipher = cipher.replaceAll("\\.|\\;|\\:|\\(|\\)|\\,|\\\"", " ");
        boolean dec = false;
        int iter = 0, correctCount;
        File f = new File("words.txt");
        while(!dec && iter < 26){
            correctCount = 0;
            cipher = shift(cipher);
            String[] splitted = cipher.split("\\s+"); 
            for(String split : splitted){
                try(BufferedReader r = new BufferedReader(new FileReader(f))){
                    String line = r.readLine();
                    while(line != null){
                        if(split.equals(line)){
                            correctCount++;
                            break;
                        }
                        line = r.readLine();
                    }
                }catch(java.io.IOException e){
                    System.out.println("no such file"); 
                    return null;
                }
            }
            if(correctCount > 10 || correctCount == splitted.length){
                dec = true;
                break;
            }
            iter++;
        }
        if(dec)
            return cipher;
        return "Could not find a solution";
    }

    public static void main(String[] args){
        String s = "ojXlsR";
        /*for(int i = 0; i < 26; i++){
            s = shift(s);
            System.out.println(s);
        }
        */
        System.out.println(s);
        System.out.println(decipher(s));
    }
}
