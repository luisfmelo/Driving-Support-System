package seai_server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URL;
import java.sql.Time;
import java.text.SimpleDateFormat;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Date;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.Mixer;
import javax.sound.sampled.UnsupportedAudioFileException;
import javax.swing.JOptionPane;

import seai_server.teste.setdate;
//import seai_server.player;

/*
 * The server that can be run both as a console application or a GUI
 */
public class Server {
	// a unique ID for each connection
	private static int uniqueId;
	// an ArrayList to keep the list of the Client
	private ArrayList<ClientThread> al;
	// if I am in a GUI
	private ServerGUI sg;
	// to display time
	private SimpleDateFormat sdf;
	// the port number to listen for connection
	private int port;
	// the boolean that will be turned of to stop the server
	private boolean keepGoing;
	
	public static teste SuperGui = new teste(); 
	public volatile boolean startfadiga=true;
	public String w1,w2, olhosstate, volantestate, pedalstate, speedstate, gelostate, speed,pisca, linhas;
	public int velocidade;
	public String IMAGE_URL;
	long startTime = 0;//System.nanoTime();
	long count = 0;
	long avg = 0;
	

	/*
	 *  server constructor that receive the port to listen to for connection as parameter
	 *  in console
	 */
	public Server(int port) {
		this(port, null);
	}
	
	public Server(int port, ServerGUI sg) {
		// GUI or not
		this.sg = sg;
		// the port
		this.port = port;
		// to display hh:mm:ss
		sdf = new SimpleDateFormat("HH:mm:ss");
		// ArrayList for the Client list
		al = new ArrayList<ClientThread>();
	}
	
	public void start() {
		keepGoing = true;
		/* create socket server and wait for connection requests */
		try 
		{
			// the socket used by the server
			ServerSocket serverSocket = new ServerSocket(port);
			// infinite loop to wait for connections
			while(keepGoing) 
			{
				// format message saying we are waiting
				display("Server waiting for Clients on port " + port + ".");
				
				Socket socket = serverSocket.accept();  	// accept connection
				// if I was asked to stop
				if(!keepGoing)
					break;
				ClientThread t = new ClientThread(socket);  // make a thread of it
				al.add(t);									// save it in the ArrayList
				t.start();
			}
			// I was asked to stop
			try {
				serverSocket.close();
				for(int i = 0; i < al.size(); ++i) {
					ClientThread tc = al.get(i);
					try {
					tc.sInput.close();
					tc.sOutput.close();
					tc.socket.close();
					}
					catch(IOException ioE) {
						// not much I can do
					}
				}
			}
			catch(Exception e) {
				display("Exception closing the server and clients: " + e);
			}
		}
		// something went bad
		catch (IOException e) {
            String msg = sdf.format(new Date()) + " Exception on new ServerSocket: " + e + "\n";
			display(msg);
		}
	}		
    /*
     * For the GUI to stop the server
     */
	protected void stop() {
		keepGoing = false;
		// connect to myself as Client to exit statement 
		// Socket socket = serverSocket.accept();
		try {
			new Socket("localhost", port);
		}
		catch(Exception e) {
			// nothing I can really do
		}
	}
	/*
	 * Display an event (not a message) to the console or the GUI
	 */
	private void display(String msg) {
		String time = sdf.format(new Date()) + " " + msg;
		if(sg == null)
			System.out.println(time);
		else
			sg.appendEvent(time + "\n");
	}

	// for a client who logoff using the LOGOUT message
	synchronized void remove(int id) {
		// scan the array list until we found the Id
		for(int i = 0; i < al.size(); ++i) {
			ClientThread ct = al.get(i);
			// found it
			if(ct.id == id) {
				al.remove(i);
				return;
			}
		}
	}
	
	 public static void infoBox(String infoMessage, String titleBar)
	    {
	        JOptionPane.showMessageDialog(null, infoMessage, "InfoBox: " + titleBar, JOptionPane.INFORMATION_MESSAGE);
	    }
	
	 public static Mixer mixer;
	 public static Clip clip;
	
	
	
	
	
	/*
	 *  To run as a console application just open a console window and: 
	 * > java Server
	 * > java Server portNumber
	 * If the port number is not specified 5000 is used
	 */ 
	public static void main(String[] args) {
		// start server on port 5000 unless a PortNumber is specified 
		int portNumber = 4000;
		Mixer.Info[] mixInfos = AudioSystem.getMixerInfo();
		/*
		 * for(Mixer.Info info : mixInfos){
			System.out.println(info.getName() + "------" + info.getDescription());
		}*/
		mixer = AudioSystem.getMixer(mixInfos[0]);
		
		DataLine.Info datainfo = new DataLine.Info(Clip.class, null);
		try {
			clip = (Clip)mixer.getLine(datainfo);
		} catch (LineUnavailableException lue) {
			lue.printStackTrace();
		}
		
		try {
			URL soundURL = Server.class.getResource("horn.wav");
			AudioInputStream audioStream = AudioSystem.getAudioInputStream(soundURL);
			clip.open(audioStream);
		} catch (LineUnavailableException lue) {
			lue.printStackTrace();
		} catch (UnsupportedAudioFileException uaf) {
			uaf.printStackTrace();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		clip.stop();
		
		SuperGui.setVisible(true);
		switch(args.length) {
			case 1:
				try {
					portNumber = Integer.parseInt(args[0]);
				}
				catch(Exception e) {
					System.out.println("Invalid port number.");
					System.out.println("Usage is: > java Server [portNumber]");
					
					return;
				}
			case 0:
				break;
			default:
				System.out.println("Usage is: > java Server [portNumber]");
				
				return;
				
		}
		// create a server object and start it
		Server server = new Server(portNumber);
		server.start();
		
		
	}

	/** One instance of this thread will run for each client */
	class ClientThread extends Thread {
		// the socket where to listen/talk
		Socket socket;
		InputStreamReader sInput;
		OutputStreamWriter sOutput;
		// my unique id (easier for deconnection)
		int id;
		// the Username of the Client
		String username;
		// the only type of message a will receive
		//ChatMessage cm;
		// the date I connect
		String date;

		// Constructore
		ClientThread(Socket socket) {
			// a unique id
			id = ++uniqueId;
			this.socket = socket;
			/* Creating both Data Stream */
			System.out.println("Thread trying to create Object Input/Output Streams");
			try
			{
				// create output first
				sOutput = new OutputStreamWriter(socket.getOutputStream());
				sInput  = new InputStreamReader(socket.getInputStream());
			}
			catch (IOException e) {
				display("Exception creating new Input/output Streams: " + e);
				return;
			}

            date = new Date().toString() + "\n";
            System.out.println(date);
		}

		// what will run forever
		public void run() {
			// to loop until LOGOUT
			boolean keepGoing = true;
			boolean started = true;
			
			while(keepGoing) {
				//startfadiga=true;
				//(new Thread(new fadiga())).start();
				// read a String (which is an object)
				startTime = System.nanoTime();
				
				if ( SuperGui.getFadigaState() && !started && !SuperGui.getButtonState()){
					display("Started");
					clip.start();
					clip.loop(1000);
					started = true;
				}
				if ( !SuperGui.getFadigaState() || SuperGui.getButtonState() ){
					started = false;
					clip.stop();
				}
				display("FADIGA: " + SuperGui.getFadigaState());

				BufferedReader br = new BufferedReader(sInput);
				char[] buffer = new char[300];
				try {
					int count = br.read(buffer, 0, 300);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				System.out.print((this.id));
				System.out.println(": " + String.valueOf(buffer) );
				
				//display(String.valueOf(buffer));
				
				//Server.infoBox(String.valueOf(buffer),"WARNINGS");

				

				
////////////////////////////////////DETEÇAO DE LINHAS//////////////////////////////////////////////////////
				
					if(String.valueOf(buffer[0]).equals("L")){
						
						if(String.valueOf(buffer[1]).equals("e")){
						//SuperGui.changeRight(false);
						//SuperGui.changeLeft(true);
						System.out.print("\nesquerda\n");
						linhas = "esq";
						}
				
						else if(String.valueOf(buffer[1]).equals("d")){
							//SuperGui.changeRight(true);
							//SuperGui.changeLeft(false);
							System.out.print("\ndireita\n");
							linhas = "dir";
						}	
					
						else if(String.valueOf(buffer[1]).equals("c")){
							//SuperGui.changeRight(false);
							//SuperGui.changeLeft(false);
							linhas = "cen";
						}
				
				}		
						
///////////////////////////////////////SENSORES//////////////////////////////////////////////////////
				
					//String Split
					String s = String.valueOf(buffer);
					
					
					if (s.contains("-")) {
					    // Split it.
						String[] parts = s.split("-");

						//boolean forclose=false;
						
						for(int i=0; /*parts[i]!=null && !parts[i].isEmpty()*/i<=(parts.length - 1) ; i++)
						{
							
							if(s.length()!=300)
								break;
							/*if(parts[i]!=null && !parts[i].isEmpty())
								forclose=true;*/
							String part = parts[i];
							if (part.length()==1)
								part = part + "x";
							
							System.out.print("\n" + "for" + i + part +  "\n");
						
							//String speed;
						String humidity;
						String temperature;
						
						//if(String.valueOf(buffer[0]).equals("c") && String.valueOf(buffer[1]).equals("2")){
							
							
	//////////////////////////////Velocidade//////////////////////////////////////////////////////////		
							if (part.startsWith("S") && part!=null && !part.isEmpty())
							{
							
								char[] buf = part.toCharArray();
								//Character[] charObjectArray = ArrayUtils.toObject(charArray);
							
							speed = String.valueOf(buf[1]) + String.valueOf(buf[2]) + String.valueOf(buf[3]);
							
							if(String.valueOf(buf[1]).equals("0")){
								speed = String.valueOf(buf[2]) + String.valueOf(buf[3]);
							
							SuperGui.setSpeed(speed);
							//System.out.print(SuperGui.getSpeed());
							}
							
							if(String.valueOf(buf[1]).equals("0") && String.valueOf(buf[2]).equals("0")){
								speed = String.valueOf(buf[3]);
							
							SuperGui.setSpeed(speed);
							//System.out.print(SuperGui.getSpeed());
							}
							}
							//System.out.println("init: " + part.charAt(0));
	//////////////////////////////Temperatura//////////////////////////////////////////////////////////	
							if (part.startsWith("T") && part!=null && !part.isEmpty())
							{
								char[] buf = part.toCharArray();
								System.out.println("T: " + String.valueOf(buf[2]) + String.valueOf(buf[3]));
	
							//temperature = String.valueOf(buffer[6]) + String.valueOf(buffer[7])+ String.valueOf(buffer[8]);
							if(String.valueOf(buf[1]).equals("+")){
								temperature = String.valueOf(buf[2]) + String.valueOf(buf[3]);
								SuperGui.setTemperature(temperature);
									if(String.valueOf(buf[2]).equals("0")){
										temperature = String.valueOf(buf[3]);
										SuperGui.setTemperature(temperature);
									}
							}
							else{
								temperature = "-" + String.valueOf(buf[2])+ String.valueOf(buf[3]);
								
									if(String.valueOf(buf[2]).equals("0")){
										temperature = "-" + String.valueOf(buf[3]);
										SuperGui.setTemperature(temperature);
									}
							}
							
	/////////////////////////////GELO/////////////////////////////////////////////////////////////////
							if(Integer.parseInt(temperature) >= 22)
								SuperGui.changeGelo(true);
							else
								SuperGui.changeGelo(false);
							}

	//////////////////////////////Humidade//////////////////////////////////////////////////////////						
							if (part.startsWith("H") && part!=null && !part.isEmpty())
							{
								char[] buf = part.toCharArray();
							humidity = String.valueOf(buf[1]) + String.valueOf(buf[2]);
							SuperGui.setHumidity(humidity);
							//System.out.print(SuperGui.getHumidity());
							
							
							/*if(String.valueOf(buffer[2]).equals("g")){
								SuperGui.changeGelo(true);
								System.out.print("\ngelo\n");
								 //SuperGui.playAlarm();
								}
							else if (String.valueOf(buffer[2]).equals("x"))
								SuperGui.changeGelo(SuperGui.getGeloState());
							
							else if (String.valueOf(buffer[2]).equals("n"))
								SuperGui.changeGelo(false);
							
							else
								SuperGui.changeGelo(SuperGui.getGeloState());
							
							gelostate = String.valueOf(SuperGui.getGeloState());*/
							}
	/////////////////////////////Chuva////////////////////////////////////////////////////////////////					
							if (part.startsWith("R") && part!=null && !part.isEmpty())
							{
								char[] buf = part.toCharArray();
							w1 = String.valueOf(buf[1]);
							}
	/////////////////////////////Volante//////////////////////////////////////////////////////////////						
							if (part.startsWith("V") && part!=null && !part.isEmpty())
							{
								char[] buf = part.toCharArray();
							volantestate = String.valueOf(buf[1]);
							}
	////////////////////////////Pedal/////////////////////////////////////////////////////////////////					
							if (part.startsWith("P") && part!=null && !part.isEmpty())
							{
								char[] buf = part.toCharArray();
							pedalstate = String.valueOf(buf[1]);
							}
							
	////////////////////////////Batimento///////////////////////////////////////////////////////////////
							if (part.startsWith("B") && part!=null && !part.isEmpty())
							{
								char[] buf = part.toCharArray();
								
								if (String.valueOf(buf[1]).equals("1"))
									SuperGui.changeHeartRate(true);
								else
									SuperGui.changeHeartRate(false);
							}
							//B0 se normal
							//B1 se irregular
							
							if(part.startsWith("I") && part!=null && !part.isEmpty()){
								char[] buf = part.toCharArray();
								pisca = String.valueOf(buf[1]);							
							}
							
							
						}
						
						}
						
					System.out.println("Linhas:" + linhas + "\nPiscas:" + pisca);
					if(linhas == "dir" && !String.valueOf(pisca).equals("d")){
						SuperGui.changeRight(true);
						SuperGui.changeLeft(false);
					}
					
					else if(linhas == "esq" && !String.valueOf(pisca).equals("e")){
						SuperGui.changeRight(false);
						SuperGui.changeLeft(true);
					}
					
					else{
						SuperGui.changeRight(false);
						SuperGui.changeLeft(false);
					}
					
////////////////////////////// Luminosidade - Faróis //////////////////////////////////////////////////////////////
					
					if(String.valueOf(buffer[0]).equals("F") ){	
						w2 = String.valueOf(buffer[1]);
							if(String.valueOf(buffer[2]).equals("a")){
								SuperGui.setLightsAuto(true);
								SuperGui.setLightsManual(false);
							}
							else if(String.valueOf(buffer[2]).equals("m")){
								SuperGui.setLightsAuto(false);
								SuperGui.setLightsManual(true);
							}
								
					}
					
					
					
					String w = w2 + w1;
					if((!String.valueOf(w).equals("ds")) && (!String.valueOf(w).equals("dr")) && (!String.valueOf(w).equals("ns")) && (!String.valueOf(w).equals("nr"))){
						w = SuperGui.getWeather();
						if (w!=null && !w.isEmpty()){
							SuperGui.changeWeather(w);
							//System.out.print(SuperGui.getWeather());
						}
					}
					SuperGui.changeWeather(w);
					//System.out.print(SuperGui.getWeather());	
					
					
/////////////////////////////// OLHOS ///////////////////////////////////////////////////////////////////////
					
					if(String.valueOf(buffer[0]).equals("O")){	
						if(String.valueOf(buffer[1]).equals("0") || String.valueOf(buffer[1]).equals("1") || String.valueOf(buffer[1]).equals("2"))
						olhosstate = String.valueOf(buffer[1]);
					}
					
					
//////////////////////////////////DETEÇAO DE FADIGA/////////////////////////////////////////////////////////
		
		boolean fadigaState = true;
		
		
		//SuperGui.setFadiga(false);
		
	/*	if(real_speed >= 70){
			fadigaState = true;
			System.out.print("\n" + "entra no ciclo\n");
		}*/
		
		
		if(fadigaState){			
			if (speed!=null && !speed.isEmpty()){
				int real_speed = Integer.valueOf(speed);
			
			if(real_speed < 70)
					velocidade = 3;
			if(real_speed >= 70 && real_speed < 90) 
					velocidade = 0;
			if(real_speed >= 90 && real_speed < 110)
					velocidade = 1;
			if(real_speed >= 110)
					velocidade = 2;
			}
						
			if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
				String.valueOf(olhosstate).equals("0") && 
				String.valueOf(pedalstate).equals("2") && 
				String.valueOf(volantestate).equals("0") && 
				String.valueOf(w1).equals("r") && 
				String.valueOf(w2).equals("d") && 
				(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 0" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("0") && 
					String.valueOf(pedalstate).equals("2") && 
					String.valueOf(volantestate).equals("0") && 
					String.valueOf(w2).equals("n") && 
					(String.valueOf(w1).equals("r") || String.valueOf(w1).equals("s")) && 
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
					
					SuperGui.setFadiga(true);
					
					//SuperGui.playAlarm(true);
					System.out.print("\n" + "liga 1" + SuperGui.getFadigaState());
				}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("2") && 
					String.valueOf(volantestate).equals("0") &&
					(String.valueOf(w1).equals("r") || String.valueOf(w1).equals("s")) &&
					(String.valueOf(w2).equals("d") || String.valueOf(w2).equals("n")) && 
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				fadigaState = true;
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 2" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					//String.valueOf(piscaEsq).equals("1") && String.valueOf())
					//String.valueOf(piscaDir).equals("1") &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("2") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("d") &&
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 3" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("2") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("d") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 4" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("2") && 
					String.valueOf(volantestate).equals("1") &&
					(String.valueOf(w1).equals("r") || String.valueOf(w1).equals("s")) &&
					String.valueOf(w2).equals("n") &&
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 5" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("d") &&
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 6" + SuperGui.getFadigaState());
			}
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("d") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 7" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("0") &&
					(String.valueOf(w1).equals("s") || String.valueOf(w1).equals("r")) &&
					String.valueOf(w2).equals("n") &&
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 8" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("d") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga " + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("d") &&
					!SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 9" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("d") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 10" + SuperGui.getFadigaState());
			}
			
			else if(velocidade == 2 &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("d") &&
					!SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 11" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("n") &&
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 12" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("n") &&
					SuperGui.getGeloState() ){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 13" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("1") && 
					String.valueOf(volantestate).equals("1") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("n") &&
					!SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 14" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("0") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("d") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 15" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 1 ||velocidade == 2)  &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("0") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("d") &&
					!SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 16" + SuperGui.getFadigaState());
			}
			
			else if(velocidade == 2  &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("0") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("d") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 17" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("0") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("r") &&
					String.valueOf(w2).equals("n") &&
					(SuperGui.getGeloState() || !SuperGui.getGeloState())){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 18" + SuperGui.getFadigaState());
			}
			
			else if((velocidade == 0 || velocidade == 1 || velocidade == 2) &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("0") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("n") &&
					SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 19" + SuperGui.getFadigaState());
			}
						
			else if((velocidade == 1 || velocidade == 2)  &&
					String.valueOf(olhosstate).equals("1") && 
					String.valueOf(pedalstate).equals("0") && 
					String.valueOf(volantestate).equals("0") &&
					String.valueOf(w1).equals("s") &&
					String.valueOf(w2).equals("n") &&
					!SuperGui.getGeloState()){
				
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 20" + SuperGui.getFadigaState());
			}
			else if(String.valueOf(olhosstate).equals("2")){
				SuperGui.setFadiga(true);
				
				//SuperGui.playAlarm(true);
				System.out.print("\n" + "liga 21" + SuperGui.getFadigaState());
			}
			
			else if(velocidade == 3){
				fadigaState = false;
				
				//SuperGui.playAlarm(true);
			}
			else{
				fadigaState = false;
			}
		}
				
		if(!fadigaState){
			SuperGui.setFadiga(false);
					
		}
				
			
					
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//String speed = String.valueOf(buffer);
				//SuperGui.setSpeed(String.valueOf(buffer));
				// Switch on the type of message receive <--------------------- NAO DA
				if ( String.valueOf(buffer).equals("exit") )
				{
					//System.out.println("Client " + Integer.toString(id) + " disconected");
					keepGoing = false;
				}
				count += 1000;
				avg += System.nanoTime() - startTime;
				avg /= count;
				System.out.println("TEMPO MEDIO: " + avg);
			}
			// remove myself from the arrayList containing the list of the
			// connected Clients
			remove(id);
			close();
		}
		
		
		/*public class fadiga implements Runnable {

		    public void run() {
		    	while(startfadiga){
		    		
		    		
					//velocidade
		    		if (speed!=null && !speed.isEmpty()){
					if(Integer.parseInt(speed)>=70 && Integer.parseInt(speed)<90)
					    {
						speedstate = String.valueOf("0");
					    }
					if(Integer.parseInt(speed)>=90 && Integer.parseInt(speed)<110)
					    {
						speedstate = String.valueOf("1");
					    }
					if(Integer.parseInt(speed)>=110)
					    {
						speedstate = String.valueOf("2");
					    }
		    		}    
					    //printf("%d %d %d %d %d %d %d", olhos, pedal, volante, humidade, temperatura, velocidade_real, velocidade);
					    
///////////////////////////arvore de decis�o/////////////////////////////////////////////////////////
					
					if(String.valueOf(speedstate).equals("1") || String.valueOf(speedstate).equals("2") || String.valueOf(speedstate).equals("3"))
					{
					    if(String.valueOf(olhosstate).equals("0") && String.valueOf(pedalstate).equals("2") && String.valueOf(volantestate).equals("0") && String.valueOf(w1).equals("r"))
					    {
					    	SuperGui.changeFadiga(true);
					        //return 0;
					    	startfadiga=false;
					    }
					    
					    if(String.valueOf(olhosstate).equals("1"))
					    {
					        if(String.valueOf(pedalstate).equals("2"))
					        {
					            if(String.valueOf(volantestate).equals("0"))
					            {
					            	SuperGui.changeFadiga(true);
					                //return 0;
					            	startfadiga=false;
					            }
					            if(String.valueOf(volantestate).equals("1"))
					            {
					                if(String.valueOf(w1).equals("r"))
					                {
					                	SuperGui.changeFadiga(true);
					                    //return 0;
					                	startfadiga=false;
					                }
					                if(String.valueOf(w1).equals("s"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                    	SuperGui.changeFadiga(true);
					                       //return 0;
					                    	startfadiga=false;
					                    }
					                }
					            }
					        }
					        
					        if(String.valueOf(pedalstate).equals("1"))
					        {
					            if(String.valueOf(volantestate).equals("0"))
					            {
					                if(String.valueOf(w1).equals("r"))
					                {
					                	SuperGui.changeFadiga(true);
					                    //return 0;
					                	startfadiga=false;
					                }
					                if(String.valueOf(w1).equals("s"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                    	SuperGui.changeFadiga(true);
					                       //return 0;
					                    	startfadiga=false;
					                    }
					                }
					            }
					            
					            if(String.valueOf(volantestate).equals("1"))
					            {
					                if(String.valueOf(w1).equals("r"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                    	SuperGui.changeFadiga(true);
					                        //return 0;
					                    	startfadiga=false;
					                    }
					                    if(!String.valueOf(gelostate).equals("g"))
					                    {
					                        if(String.valueOf(speedstate).equals("1") || String.valueOf(speedstate).equals("2"))
					                        {
					                        	SuperGui.changeFadiga(true);
					                            //return 0;
					                        	startfadiga=false;
					                        }
					                    }
					                }
					                if(String.valueOf(w1).equals("s"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                        if(String.valueOf(speedstate).equals("1") || String.valueOf(speedstate).equals("2"))
					                        {
					                        	SuperGui.changeFadiga(true);
					                            //return 0;
					                        	startfadiga=false;
					                        }
					                    }
					                    if(!String.valueOf(gelostate).equals("g"))
					                    {
					                        if(String.valueOf(speedstate).equals("2"))
					                        {
					                        	SuperGui.changeFadiga(true);
					                           //return 0;
					                        	startfadiga=false;
					                        }
					                    }
					                }
					            }
					        }
					        
					        if(String.valueOf(pedalstate).equals("0"))
					        {
					            if(String.valueOf(volantestate).equals("0"))
					            {
					                if(String.valueOf(w1).equals("r"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                    	SuperGui.changeFadiga(true);
					                        //return 0;
					                    	startfadiga=false;
					                    }
					                    if(!String.valueOf(gelostate).equals("g"))
					                    {
					                        if(String.valueOf(speedstate).equals("1") || String.valueOf(speedstate).equals("2"))
					                        {
					                        	SuperGui.changeFadiga(true);
					                            //return 0;
					                        	startfadiga=false;
					                        }
					                    }
					                }
					                if(String.valueOf(w1).equals("s"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                        if(String.valueOf(speedstate).equals("2"))
					                        {
					                        	SuperGui.changeFadiga(true);
					                            //return 0;
					                        	startfadiga=false;
					                        }
					                    }
					                }
					            }
					            if(String.valueOf(volantestate).equals("1"))
					            {
					                if(String.valueOf(w1).equals("r"))
					                {
					                    if(String.valueOf(gelostate).equals("g"))
					                    {
					                        if(String.valueOf(speedstate).equals("2"))
					                        {
					                        	SuperGui.changeFadiga(true);
					                            //return 0;
					                        	startfadiga=false;
					                        }
					                    }
					                }
					            }
					        }
					    }
					    if(String.valueOf(olhosstate).equals("2"))
					    {
					    	SuperGui.changeFadiga(true);
					        //return 0;
					    	startfadiga=false;
					    	System.out.print("\nolhos = " + olhosstate);
					    }
					    else{
					    SuperGui.changeFadiga(false);
					      //return 0;
					    startfadiga=false;
					    }
					    
					}
					else
					{
						SuperGui.changeFadiga(false);
						startfadiga=false;
					}			
					
			    
		    	}
		    	System.out.print("\nfadiga = " + SuperGui.getFadigaState());
		    	startfadiga=true;
		    }

		}*/
		
		// try to close everything
		private void close() {
			// try to close the connection
			try {
				if(sOutput != null) sOutput.close();
			}
			catch(Exception e) {}
			try {
				if(sInput != null) sInput.close();
			}
			catch(Exception e) {};
			try {
				if(socket != null) socket.close();
			}
			catch (Exception e) {}
		}
		
	}
	
	
	
}

