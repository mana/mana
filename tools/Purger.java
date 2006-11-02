/*
 * Purger (c) 2006 Eugenio Favalli
 * License: GPL, v2 or later
 */
 
import java.io.*; 
import java.text.*;
import java.util.*;

 public class Purger {

	public static void main(String[] args) {
		if (args.length != 2) {
			System.out.println(
				"Usage: java Purger <folder> <date>\n" +
				" - folder: is the path to account.txt and athena.txt files.\n" +
				" - date: accounts created before this date will be purged (dd/mm/yy).");
			return;
		}
		
		int accounts = 0;
		int characters = 0;
		int deletedCharacters = 0;
		Vector activeAccounts = new Vector();
		
		File oldAccount = new File(args[0] + "account.txt");
		File oldAthena = new File(args[0] + "athena.txt");
		File newAccount = new File(args[0] + "account.txt.new");
		File newAthena = new File(args[0] + "athena.txt.new");
		
		DateFormat dateFormat = new SimpleDateFormat("dd/MM/yy");
		Date purgeDate;
		try {
			purgeDate = dateFormat.parse(args[1]);
		}
		catch (ParseException e) {
     			System.out.println("ERROR: Wrong date format.");
			return;
		}
		
		String line;
		try {
			FileInputStream fin =  new FileInputStream(oldAccount);
			BufferedReader input = new BufferedReader(
					new InputStreamReader(fin));
			FileOutputStream fout = new FileOutputStream(newAccount);
            PrintStream output = new PrintStream(fout);
			
			while ((line = input.readLine()) != null) {
				boolean copy = false;
				String[] fields = line.split("\t");
				// Check if we're reading a comment or the last line
				if (line.substring(0, 2).equals("//") || fields[1].charAt(0) == '%') {
					copy = true;
				}
				else {
					// Server accounts should not be purged
					if (!fields[4].equals("S")) {
						accounts++;
						dateFormat = new SimpleDateFormat("yyyy-MM-dd");						
						try {
							Date date = dateFormat.parse(fields[3]);
							if (date.after(purgeDate)) {
								activeAccounts.add(fields[0]);
								copy = true;
							}
						}
						catch (ParseException e) {
							System.out.println(
								"ERROR: Wrong date format in account.txt. ("
								+ accounts + ": " + line + ")");
							//return;
						}
						catch (Exception e) {
							e.printStackTrace();
							return;
						}
					}
					else {
						copy = true;
					}					
				}
				if (copy) {
					try {
						output.println(line);
					}
					catch (Exception e) {
						System.err.println("ERROR: Unable to write file.");
					}
				}
			}
		}
		catch (FileNotFoundException e ) {
			System.out.println(
				"ERROR: file " + oldAccount.getAbsolutePath() + " not found.");
			return;
		}
		catch (Exception e) {
			System.out.println("ERROR: unable to process account.txt");
			e.printStackTrace();
			return;
		}
		
		input.close();
		output.close();
		
		try {
			FileInputStream fin =  new FileInputStream(oldAthena);
			BufferedReader input = new BufferedReader(
					new InputStreamReader(fin));
			FileOutputStream fout = new FileOutputStream(newAthena);
            PrintStream output = new PrintStream(fout);
			
			while ((line = input.readLine()) != null) {
				boolean copy = false;
				String[] fields = line.split("\t");
				// Check if we're reading a comment or the last line
				if (line.substring(0, 2).equals("//")
						|| fields[1].charAt(0) == '%') {
					copy = true;
				}
				else {
					characters++;
					String id = fields[1].substring(0, fields[1].indexOf(','));
					if (activeAccounts.contains(id)) {
						copy = true;
					}
					else {
						deletedCharacters++;
					}
				}
				if (copy) {
					output.println(line);
				}
			}
		}
		catch (FileNotFoundException e ) {
			System.out.println(
				"ERROR: file " + oldAthena.getAbsolutePath() + " not found.");
			return;
		}
		catch (Exception e) {
			System.out.println("ERROR: unable to process athena.txt");
			e.printStackTrace();
			return;
		}
		
		input.close();
		output.close();
		
		System.out.println(
			"Removed " + (accounts - activeAccounts.size()) + "/" +
			accounts + " accounts.");
		System.out.println(
			"Removed " + deletedCharacters + "/"
			+ characters + " characters.");
	}
	
}