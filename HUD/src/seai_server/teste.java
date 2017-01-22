package seai_server;

import java.awt.Color;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.Toolkit;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.File;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;

public class teste extends javax.swing.JFrame {

	/**
	 * 
	 */
	private static final long serialVersionUID = -830285773342925783L;
	public static String mensagem;
	final String DEGREES = "\u00b0";
	private static boolean gelo = false;
	private static boolean tired = false;
	private static boolean left = false;
	private static boolean right = false;
	private static boolean heart = false;
	private static boolean active = false;
	private static boolean selected = false;
	private static boolean auto = false;
	private static boolean manual = false;
	private String vel;
	private String hum;
	private String temp;
	private String weather;
	private JLabel fadiga;
	private JLabel labelLeft;
	private JLabel humidity;
	private JLabel temperatura;
	private JLabel DegreesCelsius;
	private JLabel kmh;
	private JLabel data;
	private JLabel labelRight;
	private JLabel inicial;
	private JLabel labelGelo;
	private JLabel labelSun;
	private JLabel labelMoon;
	private JLabel labelRainyDay;
	private JLabel labelRainyNight;
	private JLabel labelHeart;
	private JLabel box;
	private JLabel lblTemperatura;
	private JLabel lblHumidade;
	private JLabel lblVelocidade;
	private JLabel labelAuto;
	private JLabel labelManual;
	private JToggleButton button;
	// private AudioInputStream inputStream =
	// AudioSystem.getAudioInputStream(new File("./src/Imagens/alarm.au"));
	// private Clip myclip = AudioSystem.getClip();

	private final double WIDTH = Toolkit.getDefaultToolkit().getScreenSize().getWidth();
	private final double HEIGHT = Toolkit.getDefaultToolkit().getScreenSize().getHeight();

	/**
	 * Launch the application.
	 */
	public static void main() {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					new teste().setVisible(true);

				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the application.
	 */
	public teste() {
		setBackground(Color.red);
		setTitle("WARNINGS");
		initialize();
		(new Thread(new setdate())).start();

	}

	/**
	 * Initialize the contents of the frame.
	 */
	public void initialize() {
		this.setBounds(200, 200, 623, 352);
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.getContentPane().setBackground(new Color(255, 255, 255));
		getContentPane().setLayout(null);

		fadiga = new JLabel("Cuidado!! Precisa de descanso!");
		fadiga.setIcon(new ImageIcon("./src/Imagens/coffee.png"));
		fadiga.setForeground(new Color(0, 0, 0));
		fadiga.setBackground(Color.white);
		fadiga.setFont(new Font("Waree", Font.BOLD, 20));
		fadiga.setHorizontalAlignment(SwingConstants.CENTER);
		fadiga.setBounds(50, 20, 500, 65);
		getContentPane().add(fadiga);
		fadiga.setBackground(Color.white);
		fadiga.setOpaque(true);
		fadiga.setBorder(BorderFactory.createLineBorder(Color.black, 5));
		fadiga.setIconTextGap(20);
		fadiga.setVisible(false);

		inicial = new JLabel();
		inicial.setText("BOA VIAGEM");
		inicial.setIcon(new ImageIcon("./src/Imagens/car.png"));
		inicial.setForeground(new Color(0, 0, 0));
		inicial.setBackground(Color.white);
		inicial.setFont(new Font("Waree", Font.BOLD, 20));
		inicial.setHorizontalAlignment(SwingConstants.CENTER);
		inicial.setBounds(50, 20, 500, 65);
		getContentPane().add(inicial);
		inicial.setBackground(Color.white);
		inicial.setOpaque(true);
		inicial.setBorder(BorderFactory.createLineBorder(Color.black, 5));
		inicial.setIconTextGap(20);
		inicial.setVisible(true);

		lblTemperatura = new JLabel("Temperatura");
		lblTemperatura.setIcon(new ImageIcon("./src/Imagens/temperature.png"));
		lblTemperatura.setFont(new Font("Dialog", Font.BOLD, 13));
		lblTemperatura.setBackground(new Color(238, 238, 238));
		lblTemperatura.setBounds(61, 195, 139, 15);
		getContentPane().add(lblTemperatura);

		lblVelocidade = new JLabel("Velocidade");
		lblVelocidade.setIcon(new ImageIcon("./src/Imagens/speed.png"));
		lblVelocidade.setFont(new Font("Dialog", Font.BOLD, 13));
		lblVelocidade.setBackground(new Color(238, 238, 238));
		lblVelocidade.setBounds(61, 168, 139, 15);
		getContentPane().add(lblVelocidade);

		lblHumidade = new JLabel(" Humidade");
		lblHumidade.setIcon(new ImageIcon("./src/Imagens/humidity.png"));
		lblHumidade.setFont(new Font("Dialog", Font.BOLD, 13));
		lblHumidade.setBackground(new Color(238, 238, 238));
		lblHumidade.setBounds(65, 222, 139, 15);
		getContentPane().add(lblHumidade);

		kmh = new JLabel();
		kmh.setFont(new Font("Dialog", Font.BOLD, 13));
		kmh.setBackground(new Color(238, 238, 238));
		kmh.setBounds(218, 168, 70, 15);
		getContentPane().add(kmh);

		DegreesCelsius = new JLabel();
		DegreesCelsius.setFont(new Font("Dialog", Font.BOLD, 13));
		DegreesCelsius.setBackground(new Color(238, 238, 238));
		DegreesCelsius.setBounds(218, 195, 165, 15);
		getContentPane().add(DegreesCelsius);
		DegreesCelsius.setText("");

		humidity = new JLabel();
		humidity.setFont(new Font("Dialog", Font.BOLD, 13));
		humidity.setBackground(new Color(238, 238, 238));
		humidity.setBounds(218, 222, 70, 15);
		getContentPane().add(humidity);
		humidity.setText("");

		data = new JLabel();
		data.setIcon(new ImageIcon("./src/Imagens/clock1.png"));
		data.setBounds(50, 260, 426, 40);
		getContentPane().add(data);

		labelLeft = new JLabel();
		labelLeft.setIcon(new ImageIcon("./src/Imagens/arrowLeft.png"));
		labelLeft.setHorizontalAlignment(SwingConstants.CENTER);
		labelLeft.setBounds(50, 99, 216, 52);
		getContentPane().add(labelLeft);
		labelLeft.setVisible(false);
		labelLeft.setHorizontalTextPosition(JLabel.RIGHT);

		labelRight = new JLabel();
		labelRight.setIcon(new ImageIcon("./src/Imagens/arrowRight.png"));
		labelRight.setHorizontalAlignment(SwingConstants.CENTER);
		labelRight.setBounds(350, 99, 216, 52);
		getContentPane().add(labelRight);
		labelRight.setVisible(false);
		labelRight.setHorizontalTextPosition(JLabel.LEFT);

		labelGelo = new JLabel();
		labelGelo.setIcon(new ImageIcon("./src/Imagens/ice.png"));
		labelGelo.setHorizontalAlignment(SwingConstants.CENTER);
		labelGelo.setBounds(350, 150, 64, 61);
		getContentPane().add(labelGelo);
		labelGelo.setVisible(false);

		labelSun = new JLabel();
		labelSun.setIcon(new ImageIcon("./src/Imagens/sun.png"));
		labelSun.setHorizontalAlignment(SwingConstants.CENTER);
		labelSun.setBounds(355, 200, 55, 55);
		getContentPane().add(labelSun);
		labelSun.setVisible(false);

		labelMoon = new JLabel();
		labelMoon.setIcon(new ImageIcon("./src/Imagens/moon.png"));
		labelMoon.setHorizontalAlignment(SwingConstants.CENTER);
		labelMoon.setBounds(355, 200, 55, 55);
		getContentPane().add(labelMoon);
		labelMoon.setVisible(false);

		labelRainyDay = new JLabel();
		labelRainyDay.setIcon(new ImageIcon("./src/Imagens/RainyDay.png"));
		labelRainyDay.setHorizontalAlignment(SwingConstants.CENTER);
		labelRainyDay.setBounds(355, 200, 55, 55);
		getContentPane().add(labelRainyDay);
		labelRainyDay.setVisible(false);

		labelRainyNight = new JLabel();
		labelRainyNight.setIcon(new ImageIcon("./src/Imagens/RainyNight.png"));
		labelRainyNight.setHorizontalAlignment(SwingConstants.CENTER);
		labelRainyNight.setBounds(355, 200, 55, 55);
		getContentPane().add(labelRainyNight);
		labelRainyNight.setVisible(false);

		labelHeart = new JLabel();
		labelHeart.setIcon(new ImageIcon("./src/Imagens/heartbeat.png"));
		labelHeart.setHorizontalAlignment(SwingConstants.CENTER);
		labelHeart.setBounds(413, 152, 55, 55);
		getContentPane().add(labelHeart);
		labelHeart.setVisible(false);

		labelAuto = new JLabel();
		labelAuto.setIcon(new ImageIcon("./src/Imagens/auto.png"));
		labelAuto.setHorizontalAlignment(SwingConstants.CENTER);
		labelAuto.setBounds(410, 192, 65, 65);
		getContentPane().add(labelAuto);
		labelAuto.setVisible(false);

		labelManual = new JLabel();
		labelManual.setIcon(new ImageIcon("./src/Imagens/manual.png"));
		labelManual.setHorizontalAlignment(SwingConstants.CENTER);
		labelManual.setBounds(410, 192, 65, 65);
		getContentPane().add(labelManual);
		labelManual.setVisible(false);

		box = new JLabel();
		box.setBounds(50, 160, 300, 86);
		getContentPane().add(box);
		box.setBorder(BorderFactory.createLineBorder(Color.BLACK, 2));

		button = new JToggleButton("");
		button.setBounds(524, 232, 50, 50);
		button.setIcon(new ImageIcon("./src/Imagens/power-button-off.png"));
		getContentPane().add(button);
		button.addItemListener(itemListener);

	}

	public class setdate implements Runnable {

		public void run() {
			while (true) {
				LocalDateTime now = LocalDateTime.now();
				String dateformat = now.atZone(ZoneId.of("GMT")).format(DateTimeFormatter.RFC_1123_DATE_TIME);
				data.setText(dateformat);
			}
		}

	}

	ItemListener itemListener = new ItemListener() {
		public void itemStateChanged(ItemEvent itemEvent) {
			int state = itemEvent.getStateChange();
			if (state == ItemEvent.SELECTED) {
				System.out.println("Selected"); // show your message here

				// labelHeart.setVisible(getHeartRate());
				changeWeather(getWeather());
				labelGelo.setVisible(getGeloState());
				labelRight.setVisible(getRight());
				labelLeft.setVisible(getLeft());
				kmh.setVisible(true);
				humidity.setVisible(true);
				data.setVisible(true);
				DegreesCelsius.setVisible(true);
				lblTemperatura.setVisible(true);
				lblHumidade.setVisible(true);
				lblVelocidade.setVisible(true);
				inicial.setVisible(true);
				fadiga.setVisible(false);
				labelAuto.setVisible(getLightsAuto());
				labelManual.setVisible(getLightsManual());

				// box.setVisible(false);
				button.setIcon(new ImageIcon("./src/Imagens/power.png"));
				// setFadiga(false);
				selected = true;
			} else {
				System.out.println("Deselected"); // remove your message

				// labelHeart.setVisible();
				/*
				 * labelRainyNight.setVisible(true);
				 * labelRainyDay.setVisible(true); labelMoon.setVisible(true);
				 * labelSun.setVisible(true);
				 */
				changeWeather(getWeather());
				labelGelo.setVisible(getGeloState());
				// labelRight.setVisible(getRight());
				// labelLeft.setVisible(getLeft());
				kmh.setVisible(true);
				humidity.setVisible(true);
				data.setVisible(true);
				DegreesCelsius.setVisible(true);
				lblTemperatura.setVisible(true);
				lblHumidade.setVisible(true);
				lblVelocidade.setVisible(true);
				inicial.setVisible(true);
				fadiga.setVisible(getFadigaState());
				box.setVisible(true);
				button.setIcon(new ImageIcon("./src/Imagens/power-button-off.png"));
				selected = false;
				labelAuto.setVisible(getLightsAuto());
				labelManual.setVisible(getLightsManual());
			}
		}
	};

	public boolean getButtonState() {
		return selected;
	}

	public void playAlarm(boolean p) {
		try {
			Clip myclip = AudioSystem.getClip();
			AudioInputStream inputStream = AudioSystem.getAudioInputStream(new File("./src/Imagens/alarm.au"));
			myclip.open(inputStream);

			while (!p) {
				myclip.close();
				System.out.print("\n" + "não toca");
			}

			// player.Sound(new File("./src/Imagens/alarm.au"));
			myclip.start();
			System.out.print("\n" + "toca");
		} catch (Exception e) {
		}

	}
	
	public void setFadiga(boolean f) {
		if (!f) {
			tired = false;
			fadiga.setVisible(false);
			inicial.setVisible(true);
		} else {
			tired = true;
			if (!getButtonState()) {
				fadiga.setVisible(true);
				inicial.setVisible(false);
			}
		}

	}

	public boolean getFadigaState() {
		return tired;
	}

	public void changeLeft(boolean l) {
		// ImageIcon ice = new ImageIcon("./src/Imagens/ice-crystal.png");
		if (!l) {
			left = false;
			labelLeft.setVisible(false);
		}

		else {
			left = true;
			labelLeft.setVisible(true);
			labelLeft.setText("TURNING LEFT");
		}

	}

	public void setLightsAuto(boolean A) {
		// ImageIcon ice = new ImageIcon("./src/Imagens/ice-crystal.png");
		if (!A) {
			auto = false;
			labelAuto.setVisible(false);
		}

		else {
			auto = true;
			labelAuto.setVisible(true);
		}

	}

	public boolean getLightsAuto() {
		return auto;
	}

	public void setLightsManual(boolean M) {
		// ImageIcon ice = new ImageIcon("./src/Imagens/ice-crystal.png");
		if (!M) {
			manual = false;
			labelManual.setVisible(false);
		}

		else {
			manual = true;
			labelManual.setVisible(true);
		}

	}

	public boolean getLightsManual() {
		return manual;
	}

	public void changeRight(boolean r) {

		if (!r) {
			right = false;
			labelRight.setVisible(false);
		}

		else {
			right = true;
			labelRight.setVisible(true);
			labelRight.setText("TURNING RIGHT");
		}
	}

	public boolean getRight() {
		return right;

	}

	public boolean getLeft() {
		return left;

	}

	public void setSpeed(String speed) {
		vel = speed;
		kmh.setText(speed + "Km/h");
	}

	public String getSpeed() {
		return vel;
	}

	public void setTemperature(String temperature) {
		temp = temperature;
		DegreesCelsius.setText(temp + DEGREES + "C");
	}

	public String getTemperature() {
		return temp;
	}

	public void setHumidity(String humidade) {
		hum = humidade;
		humidity.setText(hum + "%");
	}

	public String getHumidity() {
		return hum;
	}

	public boolean getGeloState() {
		return gelo;
	}

	public void changeGelo(boolean g) {
		if (!g) {
			gelo = false;
			labelGelo.setVisible(false);
			// inicial.setVisible(true);
		}

		else {
			gelo = true;
			labelGelo.setVisible(true);
			// labelGelo.setText("Cuidado! Precisa de Descansar!");
			// la.setVisible(false);
		}
	}

	public void changeWeather(String w) {
		weather = w;
		if (String.valueOf(weather).equals("ds")) {
			labelSun.setVisible(true);
			labelMoon.setVisible(false);
			labelRainyDay.setVisible(false);
			labelRainyNight.setVisible(false);
		}
		if (String.valueOf(weather).equals("dr")) {
			labelSun.setVisible(false);
			labelMoon.setVisible(false);
			labelRainyDay.setVisible(true);
			labelRainyNight.setVisible(false);
		}
		if (String.valueOf(weather).equals("ns")) {
			labelSun.setVisible(false);
			labelMoon.setVisible(true);
			labelRainyDay.setVisible(false);
			labelRainyNight.setVisible(false);
		}
		if (String.valueOf(weather).equals("nr")) {
			labelSun.setVisible(false);
			labelMoon.setVisible(false);
			labelRainyDay.setVisible(false);
			labelRainyNight.setVisible(true);
		}

	}

	public String getWeather() {
		return weather;

	}

	public void changeHeartRate(boolean h) {
		if (!h) {
			heart = false;
			labelHeart.setVisible(false);
		} else {
			heart = true;
			labelHeart.setVisible(true);
		}

	}

	public boolean getHeartRate() {
		return heart;
	}

}
