import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.jogamp.opengl.GL2ES3;
import com.jogamp.opengl.GL3;

public class PlotMilliseconds extends Plot {
	
	enum Mode {SHOWS_TIMESTAMPS, SHOWS_SECONDS, SHOWS_MINUTES, SHOWS_HOURS};
	Mode xAxisMode;
	
	DatasetsController datasetsController;
	
	// for non-cached mode
	FloatBuffer   bufferX;
	FloatBuffer[] buffersY;
	
	// for cached mode
	DrawCallData draw1 = new DrawCallData();
	DrawCallData draw2 = new DrawCallData();
	int[]     fbHandle;
	int[]     texHandle;
	boolean   cacheIsValid;
	List<Dataset> previousNormalDatasets;
	List<Dataset.Bitfield.State> previousEdgeStates;
	List<Dataset.Bitfield.State> previousLevelStates;
	long          previousPlotMinX;
	long          previousPlotMaxX;
	float         previousPlotMinY;
	float         previousPlotMaxY;
	int           previousPlotWidth;
	int           previousPlotHeight;
	long          previousPlotDomain;
	float         previousLineWidth;
	long          previousMinSampleNumber;
	long          previousMaxSampleNumber;
	
	StorageTimestamps.Cache timestampsCache;
	
	/**
	 * Step 1: (Required) Calculate the domain and range of the plot.
	 * 
	 * @param endTimestamp      Timestamp corresponding with the right edge of a time-domain plot. NOTE: this might be in the future!
	 * @param endSampleNumber   Sample number corresponding with the right edge of a time-domain plot. NOTE: this sample might not exist yet!
	 * @param zoomLevel         Current zoom level. 1.0 = no zoom.
	 * @param datasets          Normal/edge/level datasets to acquire from.
	 * @param timestampCache    Place to cache timestamps.
	 * @param duration          The number of milliseconds to acquire, before applying the zoom factor.
	 * @param cachedMode        True to enable the cache.
	 * @param showTimestamps    True if the x-axis shows timestamps, false if the x-axis shows elapsed time.
	 */
	@Override public void initialize(long endTimestamp, long endSampleNumber, double zoomLevel, DatasetsInterface datasets, StorageTimestamps.Cache timestampsCache, long duration, boolean cachedMode, boolean showTimestamps) {
		
		this.datasets = datasets;
		this.timestampsCache = timestampsCache;
		this.cachedMode = cachedMode;
		xAxisMode = showTimestamps ? Mode.SHOWS_TIMESTAMPS : Mode.SHOWS_SECONDS;
		xAxisTitle = showTimestamps ? "Time" : "Time Elapsed (Seconds)";
		
		// calculate the domain, ensuring it's >= 1ms
		plotDomain = (long) Math.ceil(duration * zoomLevel);
		plotMaxX = endTimestamp;
		plotMinX = plotMaxX - plotDomain;
		if(plotMinX == plotMaxX) {
			plotMinX = plotMaxX - 1;
			plotDomain = plotMaxX - plotMinX;
		}

		// determine which samples to acquire
		datasetsController = datasets.hasAnyType() ? datasets.connection.datasets : null;
		int sampleCount = datasetsController == null ? 0 : datasetsController.getSampleCount();
		if(sampleCount > 0) {
			maxSampleNumber = datasetsController.getClosestSampleNumberAfter(plotMaxX);
			minSampleNumber = datasetsController.getClosestSampleNumberAtOrBefore(plotMinX, sampleCount - 1);
	
			if(minSampleNumber < 0)
				minSampleNumber = 0;
			
			plotSampleCount = maxSampleNumber - minSampleNumber + 1;
		} else {
			// exit if there are no samples to acquire
			maxSampleNumber = -1;
			minSampleNumber = -1;
			plotSampleCount = 0;
			samplesMinY = -1;
			samplesMaxY =  1;
			return;
		}
		
		// get the range
		float[] range = datasets.getRange((int) minSampleNumber, (int) maxSampleNumber);
		samplesMinY = range[0];
		samplesMaxY = range[1];
		
		// determine the x-axis title if showing time elapsed
		if(!showTimestamps) {
			long leftMillisecondsElapsed = plotMinX - datasetsController.getFirstTimestamp();
			long hours = leftMillisecondsElapsed / 3600000; leftMillisecondsElapsed %= 3600000;
			long minutes = leftMillisecondsElapsed / 60000; leftMillisecondsElapsed %= 60000;
			xAxisMode = (hours == 0 && minutes == 0) ? Mode.SHOWS_SECONDS :
			            (hours == 0) ?                 Mode.SHOWS_MINUTES :
			                                           Mode.SHOWS_HOURS;
			
			long rightMillisecondsElapsed = plotMaxX - datasetsController.getFirstTimestamp();
			hours = rightMillisecondsElapsed / 3600000; rightMillisecondsElapsed %= 3600000;
			minutes = rightMillisecondsElapsed / 60000; rightMillisecondsElapsed %= 60000;
			if(hours == 0 && minutes != 0 && xAxisMode == Mode.SHOWS_SECONDS)
				xAxisMode = Mode.SHOWS_MINUTES;
			if(hours != 0)
				xAxisMode = Mode.SHOWS_HOURS;
			
			xAxisTitle = (xAxisMode == Mode.SHOWS_HOURS)   ? "Time Elapsed (HH:MM:SS.SSS)" :
			             (xAxisMode == Mode.SHOWS_MINUTES) ? "Time Elapsed (MM:SS.SSS)" :
			                                                 "Time Elapsed (Seconds)";
		}
		
	}
	
	// steps 2 and 3 are handled by the Plot class
	
	/**
	 * Step 4: Get the x-axis divisions.
	 * 
	 * @param gl           The OpenGL context.
	 * @param plotWidth    The width of the plot region, in pixels.
	 * @return             A Map where each value is a string to draw on screen, and each key is the pixelX location for it (0 = left edge of the plot)
	 */
	@Override public Map<Float, String> getXdivisions(GL2ES3 gl, float plotWidth) {
		
		if(xAxisMode == Mode.SHOWS_TIMESTAMPS)
			return ChartUtils.getTimestampDivisions(gl, plotWidth, plotMinX, plotMaxX);
			
		Map<Float, String> divisions = new HashMap<Float, String>();
		
		// sanity check
		if(plotWidth < 1)
			return divisions;
		
		// determine how many divisions can fit on screen
		long firstTimestamp = maxSampleNumber >= 0 ? datasetsController.getFirstTimestamp() : 0;
		long hours = 0;
		long minutes = 0;
		long seconds = 0;
		long milliseconds = 0;
		
		long leftMillisecondsElapsed = plotMinX - firstTimestamp;
		boolean negative = leftMillisecondsElapsed < 0;
		if(negative) leftMillisecondsElapsed *= -1;
		hours = leftMillisecondsElapsed / 3600000; leftMillisecondsElapsed %= 3600000;
		minutes = leftMillisecondsElapsed / 60000; leftMillisecondsElapsed %= 60000;
		seconds = leftMillisecondsElapsed / 1000;  leftMillisecondsElapsed %= 1000;
		milliseconds = leftMillisecondsElapsed;
		leftMillisecondsElapsed = plotMinX - firstTimestamp;
		String leftLabel = (xAxisMode == Mode.SHOWS_HOURS)   ? String.format("%s%02d:%02d:%02d.%03d", negative ? "-" : "", hours, minutes, seconds, milliseconds) :
		                   (xAxisMode == Mode.SHOWS_MINUTES) ? String.format("%s%02d:%02d.%03d",      negative ? "-" : "",        minutes, seconds, milliseconds) :
		                                                       String.format("%s%02d.%03d",           negative ? "-" : "",                 seconds, milliseconds);

		long rightMillisecondsElapsed = plotMaxX - firstTimestamp;
		hours = rightMillisecondsElapsed / 3600000; rightMillisecondsElapsed %= 3600000;
		minutes = rightMillisecondsElapsed / 60000; rightMillisecondsElapsed %= 60000;
		seconds = rightMillisecondsElapsed / 1000;  rightMillisecondsElapsed %= 1000;
		milliseconds = rightMillisecondsElapsed;
		rightMillisecondsElapsed = plotMaxX - firstTimestamp;
		String rightLabel = (xAxisMode == Mode.SHOWS_HOURS)   ? String.format("%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds) :
		                    (xAxisMode == Mode.SHOWS_MINUTES) ? String.format("%02d:%02d.%03d",             minutes, seconds, milliseconds) :
		                                                        String.format("%02d.%03d",                           seconds, milliseconds);
		
		float maxLabelWidth = Float.max(OpenGL.smallTextWidth(gl, leftLabel), OpenGL.smallTextWidth(gl, rightLabel));
		float padding = maxLabelWidth / 2f;
		int divisionCount = (int) (plotWidth / (maxLabelWidth + padding));
		
		// determine where the divisions should occur
		long millisecondsOnScreen = plotMaxX - plotMinX;
		long millisecondsPerDivision = (long) Math.ceil((double) millisecondsOnScreen / (double) divisionCount);
		if(millisecondsPerDivision == 0)
			millisecondsPerDivision = 1;
		
		long firstDivisionMillisecondsElapsed = leftMillisecondsElapsed;
		if(millisecondsPerDivision < 1000) {
			// <1s per div, so use 1/2/5/10/20/50/100/200/250/500/1000ms per div, relative to the nearest second
			millisecondsPerDivision = (millisecondsPerDivision <= 1)   ? 1 :
			                          (millisecondsPerDivision <= 2)   ? 2 :
			                          (millisecondsPerDivision <= 5)   ? 5 :
			                          (millisecondsPerDivision <= 10)  ? 10 :
			                          (millisecondsPerDivision <= 20)  ? 20 :
			                          (millisecondsPerDivision <= 50)  ? 50 :
			                          (millisecondsPerDivision <= 100) ? 100 :
			                          (millisecondsPerDivision <= 200) ? 200 :
			                          (millisecondsPerDivision <= 250) ? 250 :
			                          (millisecondsPerDivision <= 500) ? 500 :
			                                                             1000;
			firstDivisionMillisecondsElapsed = (leftMillisecondsElapsed / 1000 * 1000) + (long) Math.ceil((double) (leftMillisecondsElapsed % 1000) / (double) millisecondsPerDivision) * millisecondsPerDivision;
		} else if(millisecondsPerDivision < 60000) {
			// <1m per div, so use 1/2/5/10/15/20/30/60s per div, relative to the nearest minute
			millisecondsPerDivision = (millisecondsPerDivision <= 1000)  ? 1000 :
			                          (millisecondsPerDivision <= 2000)  ? 2000 :
			                          (millisecondsPerDivision <= 5000)  ? 5000 :
			                          (millisecondsPerDivision <= 10000) ? 10000 :
			                          (millisecondsPerDivision <= 15000) ? 15000 :
			                          (millisecondsPerDivision <= 20000) ? 20000 :
			                          (millisecondsPerDivision <= 30000) ? 30000 :
			                                                               60000;
			firstDivisionMillisecondsElapsed = (leftMillisecondsElapsed / 60000 * 60000) + (long) Math.ceil((double) (leftMillisecondsElapsed % 60000) / (double) millisecondsPerDivision) * millisecondsPerDivision;
		} else if(millisecondsPerDivision < 3600000) {
			// <1h per div, so use 1/2/5/10/15/20/30/60m per div, relative to the nearest hour
			millisecondsPerDivision = (millisecondsPerDivision <= 60000)   ? 60000 :
			                          (millisecondsPerDivision <= 120000)  ? 120000 :
			                          (millisecondsPerDivision <= 300000)  ? 300000 :
			                          (millisecondsPerDivision <= 600000)  ? 600000 :
			                          (millisecondsPerDivision <= 900000)  ? 900000 :
			                          (millisecondsPerDivision <= 1200000) ? 1200000 :
			                          (millisecondsPerDivision <= 1800000) ? 1800000 :
			                                                                 3600000;
			firstDivisionMillisecondsElapsed = (leftMillisecondsElapsed / 3600000 * 3600000) + (long) Math.ceil((double) (leftMillisecondsElapsed % 3600000) / (double) millisecondsPerDivision) * millisecondsPerDivision;
		} else if(millisecondsPerDivision < 86400000) {
			// <1d per div, so use 1/2/3/4/6/8/12/24 hours per div, relative to the nearest day
			millisecondsPerDivision = (millisecondsPerDivision <= 3600000)  ? 3600000 :
			                          (millisecondsPerDivision <= 7200000)  ? 7200000 :
			                          (millisecondsPerDivision <= 10800000) ? 10800000 :
			                          (millisecondsPerDivision <= 14400000) ? 14400000 :
			                          (millisecondsPerDivision <= 21600000) ? 21600000 :
			                          (millisecondsPerDivision <= 28800000) ? 28800000 :
			                          (millisecondsPerDivision <= 43200000) ? 43200000 :
			                                                                  86400000;
			firstDivisionMillisecondsElapsed = (leftMillisecondsElapsed / 86400000 * 86400000) + (long) Math.ceil((double) (leftMillisecondsElapsed % 86400000) / (double) millisecondsPerDivision) * millisecondsPerDivision;
		} else {
			// >=1d per div, so use an integer number of days, relative to the nearest day
			if(millisecondsPerDivision != 86400000)
				millisecondsPerDivision += 86400000 - (millisecondsPerDivision % 86400000);
			firstDivisionMillisecondsElapsed = (leftMillisecondsElapsed / 86400000 * 86400000) + (long) Math.ceil((double) (leftMillisecondsElapsed % 86400000) / (double) millisecondsPerDivision) * millisecondsPerDivision;
		}
		
		// populate the Map
		for(int divisionN = 0; divisionN < divisionCount; divisionN++) {
			long millisecondsElapsed = firstDivisionMillisecondsElapsed + (divisionN * millisecondsPerDivision);
			negative = millisecondsElapsed < 0;
			float pixelX = (float) (millisecondsElapsed - leftMillisecondsElapsed) / (float) millisecondsOnScreen * plotWidth;
			if(negative) millisecondsElapsed *= -1;
			hours = millisecondsElapsed / 3600000; millisecondsElapsed %= 3600000;
			minutes = millisecondsElapsed / 60000; millisecondsElapsed %= 60000;
			seconds = millisecondsElapsed / 1000;  millisecondsElapsed %= 1000;
			milliseconds = millisecondsElapsed;
			String label = (xAxisMode == Mode.SHOWS_HOURS)           ? String.format("%s%02d:%02d:%02d.%03d", negative ? "-" : "", hours, minutes, seconds, milliseconds) :
			               (xAxisMode == Mode.SHOWS_MINUTES)         ? String.format("%s%02d:%02d.%03d",      negative ? "-" : "",        minutes, seconds, milliseconds) :
			                                                           String.format("%s%01d.%03d",           negative ? "-" : "",                 seconds, milliseconds);
			if(pixelX <= plotWidth)
				divisions.put(pixelX, label);
			else
				break;
		}
		
		return divisions;
		
	}
	
	/**
	 * Step 5: Acquire the samples.
	 * 
	 * @param plotMinY      Y-axis value at the bottom of the plot.
	 * @param plotMaxY      Y-axis value at the top of the plot.
	 * @param plotWidth     Width of the plot region, in pixels.
	 * @param plotHeight    Height of the plot region, in pixels.
	 */
	@Override public void acquireSamplesNonCachedMode(float plotMinY, float plotMaxY, int plotWidth, int plotHeight) {
		
		events = new BitfieldEvents(true, true, datasets, (int) minSampleNumber, (int) maxSampleNumber);
			
		bufferX = datasetsController.getTimestampsBuffer((int) minSampleNumber, (int) maxSampleNumber, timestampsCache, plotMinX);
		
		buffersY = new FloatBuffer[datasets.normalsCount()];
		for(int datasetN = 0; datasetN < datasets.normalsCount(); datasetN++) {
			Dataset dataset = datasets.getNormal(datasetN);
			if(!dataset.isBitfield)
				buffersY[datasetN] = datasets.getSamplesBuffer(dataset, (int) minSampleNumber, (int) maxSampleNumber);
		}
		
	}
	
	/**
	 * Step 5: Acquire the samples.
	 * 
	 * @param plotMinY      Y-axis value at the bottom of the plot.
	 * @param plotMaxY      Y-axis value at the top of the plot.
	 * @param plotWidth     Width of the plot region, in pixels.
	 * @param plotHeight    Height of the plot region, in pixels.
	 */
	@Override void acquireSamplesCachedMode(float plotMinY, float plotMaxY, int plotWidth, int plotHeight) {
		
		events = new BitfieldEvents(true, true, datasets, (int) minSampleNumber, (int) maxSampleNumber);
		
		// check if the cache must be flushed
		cacheIsValid = datasets.normalDatasets.equals(previousNormalDatasets) &&
		               datasets.edgeStates.equals(previousEdgeStates) &&
		               datasets.levelStates.equals(previousLevelStates) &&
		               (plotMinY == previousPlotMinY) &&
		               (plotMaxY == previousPlotMaxY) &&
		               (plotWidth == previousPlotWidth) &&
		               (plotHeight == previousPlotHeight) &&
		               (plotMinX < previousPlotMaxX) &&
		               (plotMaxX > previousPlotMinX) &&
		               (plotDomain == previousPlotDomain) &&
		               (Theme.lineWidth == previousLineWidth) &&
		               (fbHandle != null) &&
		               (texHandle != null);
		
		// of the samples to display, some might already be in the framebuffer, so determine what subset actually needs to be drawn
		long firstSampleNumber = minSampleNumber;
		long lastSampleNumber  = maxSampleNumber;
		if(cacheIsValid) {
			if(firstSampleNumber == previousMinSampleNumber && lastSampleNumber == previousMaxSampleNumber) {
				// nothing to draw
				firstSampleNumber = lastSampleNumber;
			} else if(firstSampleNumber > previousMinSampleNumber) {
				// moving forward in time
				firstSampleNumber = previousMaxSampleNumber;
			} else if(firstSampleNumber < previousMinSampleNumber) {
				// moving backwards in time
				lastSampleNumber = previousMinSampleNumber + calculateSamplesNeededAfter(previousMinSampleNumber, plotWidth);
			} else if(firstSampleNumber == previousMinSampleNumber && lastSampleNumber > previousMaxSampleNumber) {
				// moving forward in time while x=0 is still on screen
				firstSampleNumber = previousMaxSampleNumber;
			} else {
				// moving backwards in time while x=0 is still on screen
				firstSampleNumber = lastSampleNumber;
			}
		}
		
		// the framebuffer is used as a ring buffer. since the pixels may wrap around from the right edge back to the left edge,
		// we may need to split the rendering into 2 draw calls (splitting it at the right edge of the framebuffer)
		long plotMaxMillisecondsElapsed = plotMaxX - datasetsController.getFirstTimestamp();
		long splittingTimestamp = (plotMaxMillisecondsElapsed - (plotMaxMillisecondsElapsed % plotDomain)) + datasetsController.getFirstTimestamp();

		if(firstSampleNumber == lastSampleNumber) {
			
			// nothing to draw
			draw1.enabled = false;
			draw2.enabled = false;
			
		} else if(datasetsController.getTimestamp((int) lastSampleNumber) <= splittingTimestamp || datasetsController.getTimestamp((int) firstSampleNumber) >= splittingTimestamp) {
			
			// only 1 draw call required (no need to wrap around the ring buffer)
			long leftTimestamp  = Long.max(plotMinX, datasetsController.getTimestamp((int) firstSampleNumber));
			long rightTimestamp = Long.min(plotMaxX, datasetsController.getTimestamp((int) lastSampleNumber));
			draw1.enableAndAcquire(datasets, firstSampleNumber, lastSampleNumber, leftTimestamp, rightTimestamp, plotWidth, plotHeight);
			draw2.enabled = false;
			
		} else {
			
			// to prevent a possible cache flush BETWEEN draw1 and draw2, first ask for the full range so the cache will be flushed if necessary BEFORE we prepare for draw1 and draw2
			long leftTimestamp  = Long.max(plotMinX, datasetsController.getTimestamp((int) firstSampleNumber));
			long rightTimestamp = Long.min(plotMaxX, datasetsController.getTimestamp((int) lastSampleNumber));
			draw1.enableAndAcquire(datasets, firstSampleNumber, lastSampleNumber, leftTimestamp, rightTimestamp, plotWidth, plotHeight);
			
			// 2 draw calls required because we need to wrap around the ring buffer
			long splittingSampleNumber = datasetsController.getClosestSampleNumberAfter(splittingTimestamp);
			
			leftTimestamp  = Long.max(plotMinX,           datasetsController.getTimestamp((int) firstSampleNumber));
			rightTimestamp = Long.min(splittingTimestamp, datasetsController.getTimestamp((int) splittingSampleNumber));
			draw1.enableAndAcquire(datasets, firstSampleNumber, splittingSampleNumber, leftTimestamp, rightTimestamp, plotWidth, plotHeight);
			leftTimestamp  = Long.max(splittingTimestamp, datasetsController.getTimestamp((int) splittingSampleNumber - 1));
			rightTimestamp = Long.min(plotMaxX,           datasetsController.getTimestamp((int) lastSampleNumber));
			draw2.enableAndAcquire(datasets, splittingSampleNumber - 1, lastSampleNumber, leftTimestamp, rightTimestamp, plotWidth, plotHeight);
			
		}
		
		// save current state
		previousNormalDatasets = datasets.normalDatasets;
		previousEdgeStates = datasets.edgeStates;
		previousLevelStates = datasets.levelStates;
		previousPlotMinX = plotMinX;
		previousPlotMaxX = plotMaxX;
		previousPlotMinY = plotMinY;
		previousPlotMaxY = plotMaxY;
		previousPlotWidth = plotWidth;
		previousPlotHeight = plotHeight;
		previousPlotDomain = plotDomain;
		previousLineWidth = Theme.lineWidth;
		previousMinSampleNumber = minSampleNumber;
		previousMaxSampleNumber = maxSampleNumber;
		
	}
	
	/**
	 * Calculates the (x,y,w,h) arguments for glScissor() based on what region the samples will occupy on the framebuffer.
	 * 
	 * @param firstTimestamp    The first UNIX timestamp (inclusive.)
	 * @param lastTimestamp     The last UNIX timestamp (inclusive.)
	 * @param plotWidth         Width of the plot region, in pixels.
	 * @param plotHeight        Height of the plot region, in pixels.
	 * @return                  An int[4] of {x,y,w,h}
	 */
	private int[] calculateScissorArgs(long firstTimestamp, long lastTimestamp, int plotWidth, int plotHeight) {
		
		// convert the unix timestamps into milliseconds elapsed.
		firstTimestamp -= datasetsController.getFirstTimestamp();
		lastTimestamp  -= datasetsController.getFirstTimestamp();
		
		// convert the time elapsed into a pixel number on the framebuffer, keeping in mind that it's a ring buffer
		long rbMillisecondsElapsed = firstTimestamp % plotDomain;
		int rbPixelX = (int) (rbMillisecondsElapsed * plotWidth / plotDomain);
		
		// convert the time span into a pixel count
		int pixelWidth = (int) Math.ceil((double) (lastTimestamp - firstTimestamp) * (double) plotWidth / (double) plotDomain);
		
		int[] args = new int[4];
		args[0] = rbPixelX;
		args[1] = 0;
		args[2] = pixelWidth;
		args[3] = plotHeight;

		return args;
		
	}
	
	/**
	 * We need to draw slightly more samples than theoretically required because adjacent samples can affect the edges of the glScissor'd region.
	 * 
	 * @param sampleNumber    Sample number (not the UNIX timestamp!)
	 * @param plotWidth       Width of the plot region, in pixels.
	 * @return                Number of extra samples to draw before that sample.
	 */
	private int calculateSamplesNeededBefore(long sampleNumber, int plotWidth) {
		
		double millisecondsPerPixel = (double) plotDomain / (double) plotWidth;
		long extraMillisecondsNeeded = (long) Math.ceil(millisecondsPerPixel * Theme.lineWidth);
		long requiredTimestamp = datasetsController.getTimestamp((int) sampleNumber) - extraMillisecondsNeeded;
		
		final int bufferSizeMinusOne = 100 - 1; // 10 was too small
		
		int extraSamplesNeeded = 0;
		long sampleN = sampleNumber;
		while(sampleN >= 0) {
			if(sampleN < bufferSizeMinusOne) {
				extraSamplesNeeded++;
				if(datasetsController.getTimestamp((int) sampleN) < requiredTimestamp)
					return extraSamplesNeeded;
				sampleN--;
			} else {
				FloatBuffer buffer = datasetsController.getTimestampsBuffer((int) (sampleN - bufferSizeMinusOne), (int) sampleN, timestampsCache, requiredTimestamp);
				for(int i = bufferSizeMinusOne; i >= 0; i--) {
					extraSamplesNeeded++;
					if(buffer.get(i) < 0)
						return extraSamplesNeeded;
					sampleN--;
				}
			}
		}
		
		return extraSamplesNeeded;
		
	}
	
	/**
	 * We need to draw slightly more samples than theoretically required because adjacent samples can affect the edges of the glScissor'd region.
	 * 
	 * @param sampleNumber    Sample number (not the UNIX timestamp!)
	 * @param plotWidth       Width of the plot region, in pixels.
	 * @return                Number of extra samples to draw after that sample.
	 */
	private int calculateSamplesNeededAfter(long sampleNumber, int plotWidth) {
		
		double millisecondsPerPixel = (double) plotDomain / (double) plotWidth;
		long extraMillisecondsNeeded = (int) Math.ceil(millisecondsPerPixel * Theme.lineWidth);
		long requiredTimestamp = datasetsController.getTimestamp((int) sampleNumber) + extraMillisecondsNeeded;
		
		final int bufferSizeMinusOne = 100 - 1; // 10 was too small
		
		int extraSamplesNeeded = 0;
		long sampleN = sampleNumber;
		while(sampleN <= maxSampleNumber) {
			if(sampleN + bufferSizeMinusOne > maxSampleNumber) {
				extraSamplesNeeded++;
				if(datasetsController.getTimestamp((int) sampleN) > requiredTimestamp)
					return extraSamplesNeeded;
				sampleN++;
			} else {
				FloatBuffer buffer = datasetsController.getTimestampsBuffer((int) sampleN, (int) (sampleN + bufferSizeMinusOne), timestampsCache, requiredTimestamp);
				for(int i = 0; i <= bufferSizeMinusOne; i++) {
					extraSamplesNeeded++;
					if(buffer.get(i) > 0)
						return extraSamplesNeeded;
					sampleN++;
				}
			}
		}
		
		return extraSamplesNeeded;
		
	}
	
	/**
	 * Step 6: Render the plot on screen.
	 * 
	 * @param gl             The OpenGL context.
	 * @param chartMatrix    The current 4x4 matrix.
	 * @param xPlotLeft      Bottom-left corner location, in pixels.
	 * @param yPlotBottom    Bottom-left corner location, in pixels.
	 * @param plotWidth      Width of the plot region, in pixels.
	 * @param plotHeight     Height of the plot region, in pixels.
	 * @param plotMinY       Y-axis value at the bottom of the plot.
	 * @param plotMaxY       Y-axis value at the top of the plot.
	 */
	@Override public void drawNonCachedMode(GL2ES3 gl, float[] chartMatrix, int xPlotLeft, int yPlotBottom, int plotWidth, int plotHeight, float plotMinY, float plotMaxY) {
		
		float plotRange = plotMaxY - plotMinY;
		
		// clip to the plot region
		int[] originalScissorArgs = new int[4];
		gl.glGetIntegerv(GL3.GL_SCISSOR_BOX, originalScissorArgs, 0);
		gl.glScissor(originalScissorArgs[0] + (int) xPlotLeft, originalScissorArgs[1] + (int) yPlotBottom, plotWidth, plotHeight);
		
		float[] plotMatrix = Arrays.copyOf(chartMatrix, 16);
		// adjust so: x = (x - plotMinX) / domain * plotWidth + xPlotLeft;
		// adjust so: y = (y - plotMinY) / plotRange * plotHeight + yPlotBottom;
		// edit: now doing the "x - plotMinX" part before putting data into the buffers, to improve float32 precision when x is very large
		OpenGL.translateMatrix(plotMatrix,                    xPlotLeft,                  yPlotBottom, 0);
		OpenGL.scaleMatrix    (plotMatrix, (float) plotWidth/plotDomain, (float) plotHeight/plotRange, 1);
		OpenGL.translateMatrix(plotMatrix,                            0,                    -plotMinY, 0);
		OpenGL.useMatrix(gl, plotMatrix);
		
		// draw each dataset
		if(plotSampleCount >= 2) {
			for(int i = 0; i < datasets.normalsCount(); i++) {
				
				Dataset dataset = datasets.getNormal(i);
				if(dataset.isBitfield)
					continue;
				
				OpenGL.drawLinesX_Y(gl, GL3.GL_LINE_STRIP, dataset.glColor, bufferX, buffersY[i], (int) plotSampleCount);
				
				// also draw points if there are relatively few samples on screen
				float occupiedPlotWidthPercentage = (float) (datasetsController.getTimestamp((int) maxSampleNumber) - datasetsController.getTimestamp((int) minSampleNumber)) / (float) plotDomain;
				float occupiedPlotWidth = plotWidth * occupiedPlotWidthPercentage;
				boolean fewSamplesOnScreen = (occupiedPlotWidth / plotSampleCount) > (2 * Theme.pointWidth);
				if(fewSamplesOnScreen)
					OpenGL.drawPointsX_Y(gl, dataset.glColor, bufferX, buffersY[i], (int) plotSampleCount);
				
			}
		}
		
		OpenGL.useMatrix(gl, chartMatrix);
		
		// draw any bitfield changes
		if(plotSampleCount >= 2) {
			List<BitfieldEvents.EdgeMarker>  edgeMarkers  = events.getEdgeMarkers ((connection, sampleNumber) -> (float) (connection.datasets.getTimestamp(sampleNumber) - plotMinX) / (float) plotDomain * plotWidth);
			List<BitfieldEvents.LevelMarker> levelMarkers = events.getLevelMarkers((connection, sampleNumber) -> (float) (connection.datasets.getTimestamp(sampleNumber) - plotMinX) / (float) plotDomain * plotWidth);
			ChartUtils.drawMarkers(gl, edgeMarkers, levelMarkers, xPlotLeft, yPlotBottom + plotHeight, xPlotLeft + plotWidth, yPlotBottom, -1, -1);
		}

		// stop clipping to the plot region
		gl.glScissor(originalScissorArgs[0], originalScissorArgs[1], originalScissorArgs[2], originalScissorArgs[3]);
		
	}
	
	/**
	 * Step 6: Render the plot on screen.
	 * 
	 * @param gl             The OpenGL context.
	 * @param chartMatrix    The current 4x4 matrix.
	 * @param xPlotLeft      Bottom-left corner location, in pixels.
	 * @param yPlotBottom    Bottom-left corner location, in pixels.
	 * @param plotWidth      Width of the plot region, in pixels.
	 * @param plotHeight     Height of the plot region, in pixels.
	 * @param plotMinY       Y-axis value at the bottom of the plot.
	 * @param plotMaxY       Y-axis value at the top of the plot.
	 */
	@Override public void drawCachedMode(GL2ES3 gl, float[] chartMatrix, int xPlotLeft, int yPlotBottom, int plotWidth, int plotHeight, float plotMinY, float plotMaxY) {
		
		// create the off-screen framebuffer if this is the first draw call
		if(fbHandle == null) {
			fbHandle = new int[1];
			texHandle = new int[1];
			OpenGL.createOffscreenFramebuffer(gl, fbHandle, texHandle);
		}
		
		// draw on the off-screen framebuffer
		float[] offscreenMatrix = new float[16];
		OpenGL.makeOrthoMatrix(offscreenMatrix, 0, plotWidth, 0, plotHeight, -1, 1);
		if(cacheIsValid)
			OpenGL.continueDrawingOffscreen(gl, offscreenMatrix, fbHandle, texHandle, plotWidth, plotHeight);
		else
			OpenGL.startDrawingOffscreen(gl, offscreenMatrix, fbHandle, texHandle, plotWidth, plotHeight);

		// erase the invalid parts of the framebuffer
		if(plotMinX < datasetsController.getFirstTimestamp()) {
			// if x<0 is on screen, we need to erase the x<0 region because it may have old data on it
			gl.glEnable(GL3.GL_SCISSOR_TEST);
			int[] args = calculateScissorArgs(plotMaxX, plotMaxX + plotDomain, plotWidth, plotHeight);
			gl.glScissor(args[0], args[1], args[2], args[3]);
			gl.glClearColor(0, 0, 0, 0);
			gl.glClear(GL3.GL_COLOR_BUFFER_BIT);
			gl.glDisable(GL3.GL_SCISSOR_TEST);
		}
		if(plotMaxX > datasetsController.getTimestamp((int) maxSampleNumber)) {
			// if x>maxTimestamp is on screen, we need to erase the x>maxTimestamp region because it may have old data on it
			long maxTimestamp = datasetsController.getTimestamp((int) maxSampleNumber);
			long firstTimestamp = datasetsController.getFirstTimestamp();
			gl.glEnable(GL3.GL_SCISSOR_TEST);
			int[] args = calculateScissorArgs(maxTimestamp, plotMaxX, plotWidth, plotHeight);
			gl.glScissor(args[0], args[1], args[2], args[3]);
			gl.glClearColor(0, 0, 0, 0);
			gl.glClear(GL3.GL_COLOR_BUFFER_BIT);
			if((plotMaxX - firstTimestamp) % plotDomain < (maxTimestamp - firstTimestamp) % plotDomain) {
				args = calculateScissorArgs(plotMaxX - ((plotMaxX - firstTimestamp) % plotDomain), plotMaxX, plotWidth, plotHeight);
				gl.glScissor(args[0], args[1], args[2], args[3]);
				gl.glClearColor(0, 0, 0, 0);
				gl.glClear(GL3.GL_COLOR_BUFFER_BIT);
			}
			gl.glDisable(GL3.GL_SCISSOR_TEST);
		}
		if(draw1.enabled) {
			gl.glEnable(GL3.GL_SCISSOR_TEST);
			gl.glScissor(draw1.scissorArgs[0], draw1.scissorArgs[1], draw1.scissorArgs[2], draw1.scissorArgs[3]);
			gl.glClearColor(0, 0, 0, 0);
			gl.glClear(GL3.GL_COLOR_BUFFER_BIT);
			gl.glDisable(GL3.GL_SCISSOR_TEST);
		}
		if(draw2.enabled) {
			gl.glEnable(GL3.GL_SCISSOR_TEST);
			gl.glScissor(draw2.scissorArgs[0], draw2.scissorArgs[1], draw2.scissorArgs[2], draw2.scissorArgs[3]);
			gl.glClearColor(0, 0, 0, 0);
			gl.glClear(GL3.GL_COLOR_BUFFER_BIT);
			gl.glDisable(GL3.GL_SCISSOR_TEST);
		}
		
		// adjust so: x = (x - plotMinX) / domain * plotWidth;
		// adjust so: y = (y - plotMinY) / plotRange * plotHeight;
		// edit: now doing the "x - plotMinX" part before putting data into the buffers, to improve float32 precision when x is very large
		float plotRange = plotMaxY - plotMinY;
		OpenGL.scaleMatrix    (offscreenMatrix, (float) plotWidth/plotDomain, (float) plotHeight/plotRange, 1);
		OpenGL.translateMatrix(offscreenMatrix,                            0,                    -plotMinY, 0);
		OpenGL.useMatrix(gl, offscreenMatrix);
		
		// draw each dataset
		if(plotSampleCount >= 2) {
			for(int i = 0; i < datasets.normalsCount(); i++) {
				
				Dataset dataset = datasets.getNormal(i);
				if(dataset.isBitfield)
					continue;
				
				float occupiedPlotWidthPercentage = (float) (datasetsController.getTimestamp((int) maxSampleNumber) - datasetsController.getTimestamp((int) minSampleNumber)) / (float) plotDomain;
				float occupiedPlotWidth = plotWidth * occupiedPlotWidthPercentage;
				boolean fewSamplesOnScreen = (occupiedPlotWidth / plotSampleCount) > (2 * Theme.pointWidth);
				
				if(draw1.enabled) {
					gl.glEnable(GL3.GL_SCISSOR_TEST);
					gl.glScissor(draw1.scissorArgs[0], draw1.scissorArgs[1], draw1.scissorArgs[2], draw1.scissorArgs[3]);
					OpenGL.drawLinesX_Y(gl, GL3.GL_LINE_STRIP, dataset.glColor, draw1.bufferX, draw1.buffersY[i], draw1.sampleCount);
					if(fewSamplesOnScreen)
						OpenGL.drawPointsX_Y(gl, dataset.glColor, draw1.bufferX, draw1.buffersY[i], draw1.sampleCount);
					gl.glDisable(GL3.GL_SCISSOR_TEST);
				}
				
				if(draw2.enabled) {
					gl.glEnable(GL3.GL_SCISSOR_TEST);
					gl.glScissor(draw2.scissorArgs[0], draw2.scissorArgs[1], draw2.scissorArgs[2], draw2.scissorArgs[3]);
					OpenGL.drawLinesX_Y(gl, GL3.GL_LINE_STRIP, dataset.glColor, draw2.bufferX, draw2.buffersY[i], draw2.sampleCount);
					if(fewSamplesOnScreen)
						OpenGL.drawPointsX_Y(gl, dataset.glColor, draw2.bufferX, draw2.buffersY[i], draw2.sampleCount);
					gl.glDisable(GL3.GL_SCISSOR_TEST);
				}
				
			}
		}
		
//		// draw color bars at the bottom edge of the plot to indicate draw call regions
//		OpenGL.makeOrthoMatrix(offscreenMatrix, 0, plotWidth, 0, plotHeight, -1, 1);
//		OpenGL.useMatrix(gl, offscreenMatrix);
//		float[] randomColor1 = new float[] {(float) Math.random(), (float) Math.random(), (float) Math.random(), 0.5f};
//		float[] randomColor2 = new float[] {(float) Math.random(), (float) Math.random(), (float) Math.random(), 0.5f};
//		if(draw1.enabled)
//			OpenGL.drawBox(gl, randomColor1, draw1.scissorArgs[0] + 0.5f, 0, draw1.scissorArgs[2], 10);
//		if(draw2.enabled)
//			OpenGL.drawBox(gl, randomColor2,  draw2.scissorArgs[0] + 0.5f, 0, draw2.scissorArgs[2], 10);
		
		// switch back to the screen framebuffer
		OpenGL.stopDrawingOffscreen(gl, chartMatrix);
		
		// draw the framebuffer on screen
		float startX = (float) ((plotMaxX - datasetsController.getFirstTimestamp()) % plotDomain) / plotDomain;
		OpenGL.drawRingbufferTexturedBox(gl, texHandle, xPlotLeft, yPlotBottom, plotWidth, plotHeight, startX);

		// clip to the plot region
		int[] originalScissorArgs = new int[4];
		gl.glGetIntegerv(GL3.GL_SCISSOR_BOX, originalScissorArgs, 0);
		gl.glScissor(originalScissorArgs[0] + (int) xPlotLeft, originalScissorArgs[1] + (int) yPlotBottom, plotWidth, plotHeight);
		
		// draw any bitfield changes
		if(plotSampleCount >= 2) {
			List<BitfieldEvents.EdgeMarker>  edgeMarkers  = events.getEdgeMarkers ((connection, sampleNumber) -> (connection.datasets.getTimestamp(sampleNumber) - plotMinX) / (float) plotDomain * plotWidth);
			List<BitfieldEvents.LevelMarker> levelMarkers = events.getLevelMarkers((connection, sampleNumber) -> (connection.datasets.getTimestamp(sampleNumber) - plotMinX) / (float) plotDomain * plotWidth);
			ChartUtils.drawMarkers(gl, edgeMarkers, levelMarkers, xPlotLeft, yPlotBottom + plotHeight, xPlotLeft + plotWidth, yPlotBottom, -1, -1);
		}
		
		// stop clipping to the plot region
		gl.glScissor(originalScissorArgs[0], originalScissorArgs[1], originalScissorArgs[2], originalScissorArgs[3]);
		
//		// draw the framebuffer without ringbuffer wrapping, 10 pixels above the plot
//		gl.glDisable(GL2.GL_SCISSOR_TEST);
//		OpenGL.drawTexturedBox(gl, texHandle, xPlotLeft, yPlotBottom + plotHeight + 10, plotWidth, plotHeight, 0);
//		gl.glEnable(GL2.GL_SCISSOR_TEST);
		
	}
	
	/**
	 * Checks if a tooltip should be drawn for the mouse's current location.
	 * 
	 * @param mouseX       The mouse's location along the x-axis, in pixels (0 = left edge of the plot)
	 * @param plotWidth    Width of the plot region, in pixels.
	 * @return             An object indicating if the tooltip should be drawn, for what sample number, with what label, and at what location on screen.
	 */
	@Override public TooltipInfo getTooltip(int mouseX, float plotWidth) {
		
		if(plotSampleCount == 0) {
			
			return new TooltipInfo(false, 0, "", 0);
			
		} else {
			
			long mouseTimestamp = (long) Math.round((mouseX / plotWidth) * plotDomain) + plotMinX;
			
			if(mouseTimestamp < datasetsController.getFirstTimestamp())
				return new TooltipInfo(false, 0, "", 0);
			
			long closestSampleNumberBefore = datasetsController.getClosestSampleNumberAtOrBefore(mouseTimestamp, (int) maxSampleNumber - 1);
			long closestSampleNumberAfter = closestSampleNumberBefore + 1;
			if(closestSampleNumberAfter > maxSampleNumber)
				closestSampleNumberAfter = maxSampleNumber;

			double beforeError = (double) ((mouseX / plotWidth) * plotDomain) - (double) (datasetsController.getTimestamp((int) closestSampleNumberBefore) - plotMinX);
			double afterError = (double) (datasetsController.getTimestamp((int) closestSampleNumberAfter) - plotMinX) - (double) ((mouseX / plotWidth) * plotDomain);
			
			long closestSampleNumber = (beforeError < afterError) ? closestSampleNumberBefore : closestSampleNumberAfter;
			
			String label = "";
			if(xAxisMode == Mode.SHOWS_TIMESTAMPS) {
				label = "Sample " + closestSampleNumber + "\n" + SettingsController.formatTimestampToMilliseconds(datasetsController.getTimestamp((int) closestSampleNumber));
			} else {
				long millisecondsElapsed = datasetsController.getTimestamp((int) closestSampleNumber) - datasetsController.getFirstTimestamp();
				long hours = millisecondsElapsed / 3600000; millisecondsElapsed %= 3600000;
				long minutes = millisecondsElapsed / 60000; millisecondsElapsed %= 60000;
				long seconds = millisecondsElapsed / 1000;  millisecondsElapsed %= 1000;
				long milliseconds = millisecondsElapsed;
				
				String time = (xAxisMode == Mode.SHOWS_HOURS)   ? String.format("%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds) :
				              (xAxisMode == Mode.SHOWS_MINUTES) ? String.format("%02d:%02d.%03d",             minutes, seconds, milliseconds) :
				                                                  String.format("%01d.%03d",                           seconds, milliseconds);
				
				label = "Sample " + closestSampleNumber + "\nt = " + time;
			}
			
			float pixelX = getPixelXforSampleNumber(closestSampleNumber, plotWidth);
			
			return new TooltipInfo(true, closestSampleNumber, label, pixelX);
			
		}
		
	}
	
	/**
	 * Gets the horizontal location, relative to the plot, for a sample number.
	 * 
	 * @param sampleNumber    The sample number.
	 * @param plotWidth       Width of the plot region, in pixels.
	 * @return                Corresponding horizontal location on the plot, in pixels, with 0 = left edge of the plot.
	 */
	@Override float getPixelXforSampleNumber(long sampleNumber, float plotWidth) {
		
		return (float) (datasetsController.getTimestamp((int) sampleNumber) - plotMinX) / (float) plotDomain * plotWidth;
		
	}
	
	/**
	 * Deletes the off-screen framebuffer and texture.
	 * 
	 * @param gl    The OpenGL context.
	 */
	public void freeResources(GL2ES3 gl) {
		
		if(texHandle != null)
			gl.glDeleteTextures(1, texHandle, 0);
		if(fbHandle != null)
			gl.glDeleteFramebuffers(1, fbHandle, 0);
		
		texHandle = null;
		fbHandle = null;
		
	}
	
	private class DrawCallData {
		
		boolean enabled;        // if this object contains samples to draw
		int[] scissorArgs;      // {x,y,w,h} for glScissor() when drawing
		int sampleCount;        // number of vertices
		FloatBuffer   bufferX;  // x-axis values for the vertices (common to all datasets.)
		FloatBuffer[] buffersY; // y-axis values for the vertices (one buffer per dataset.)
		
		/**
		 * Acquires samples and related data so it can be drawn later.
		 * 
		 * @param datasets             Datasets and corresponding caches to acquire from.
		 * @param firstSampleNumber    First sample number (inclusive.)
		 * @param lastSampleNumber     Last sample number (inclusive.)
		 * @param leftTimestamp        Timestamp corresponding with the left edge of the plot.
		 * @param rightTimestamp       Timestamp corresponding with the right edge of the plot.
		 * @param plotWidth            Width of the plot region, in pixels.
		 * @param plotHeight           Height of the plot region, in pixels.
		 */
		void enableAndAcquire(DatasetsInterface datasets, long firstSampleNumber, long lastSampleNumber, long leftTimestamp, long rightTimestamp, int plotWidth, int plotHeight) {
			
			enabled = true;
			sampleCount = (int) (lastSampleNumber - firstSampleNumber + 1);
			scissorArgs = calculateScissorArgs(leftTimestamp, rightTimestamp, plotWidth, plotHeight);
			
			// calculate milliseconds offset from the left edge of the plot
			long xOffset = datasetsController.getFirstTimestamp();
			xOffset += ((leftTimestamp - datasetsController.getFirstTimestamp()) / plotDomain) * plotDomain;
			
			// acquire extra samples before and after, to prevent aliasing
			firstSampleNumber -= calculateSamplesNeededBefore(firstSampleNumber, plotWidth);
			lastSampleNumber += calculateSamplesNeededAfter(lastSampleNumber, plotWidth);
			if(firstSampleNumber < 0)
				firstSampleNumber = 0;
			if(lastSampleNumber > maxSampleNumber)
				lastSampleNumber = maxSampleNumber;
			sampleCount = (int) (lastSampleNumber - firstSampleNumber + 1);
			
			// acquire the samples
			bufferX = datasetsController.getTimestampsBuffer((int) firstSampleNumber, (int) lastSampleNumber, timestampsCache, xOffset);
			buffersY = new FloatBuffer[datasets.normalsCount()];
			for(int datasetN = 0; datasetN < datasets.normalsCount(); datasetN++) {
				Dataset dataset = datasets.getNormal(datasetN);
				if(!dataset.isBitfield)
					buffersY[datasetN] = datasets.getSamplesBuffer(dataset, (int) firstSampleNumber, (int) lastSampleNumber);
			}
			
		}
		
	}

}
