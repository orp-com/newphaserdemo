/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewphaserdemoAudioProcessor::NewphaserdemoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

NewphaserdemoAudioProcessor::~NewphaserdemoAudioProcessor()
{
}

//==============================================================================
const juce::String NewphaserdemoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewphaserdemoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewphaserdemoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewphaserdemoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewphaserdemoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewphaserdemoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewphaserdemoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewphaserdemoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewphaserdemoAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewphaserdemoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewphaserdemoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    
    // ✅ Properly initialize the LFO
    auto& leftLFO = leftChain.get<1>(); // Get LFO from the chain
    auto& rightLFO = rightChain.get<1>();

    // ✅ Ensure the LFO has a valid waveform
    leftLFO.initialise([](float x) { return std::sin(x); }, 128);
    rightLFO.initialise([](float x) { return std::sin(x); }, 128);

    // ✅ Set a default frequency to avoid 0 Hz assertion
    leftLFO.setFrequency(1.0f);
    rightLFO.setFrequency(1.0f);
    
    //keeping this here to as a temp fix to avoid assertation failure (lines 107-117)
}

void NewphaserdemoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewphaserdemoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewphaserdemoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    auto& leftFilters = leftChain.get<0>();
    auto& rightFilters = rightChain.get<0>();

    auto& leftLFO = leftChain.get<1>();
    auto& rightLFO = rightChain.get<1>();
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    
}

//==============================================================================
bool NewphaserdemoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewphaserdemoAudioProcessor::createEditor()
{
    //return new NewphaserdemoAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void NewphaserdemoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewphaserdemoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.rate = apvts.getRawParameterValue("Rate")->load();
    settings.depth = apvts.getRawParameterValue("Depth")->load();
    settings.feedback = apvts.getRawParameterValue("Feedback")->load();
    settings.frequency = apvts.getRawParameterValue("Frequency")->load();
    settings.mix = apvts.getRawParameterValue("Mix")->load();
    
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout NewphaserdemoAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"Rate", 1},
                                                           "Rate",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.01f, 1.0f),
                                                           1.0f)); // Default: 1 Hz

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"Depth", 1},
                                                           "Depth",
                                                           juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                                                           0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"Feedback", 1},
                                                           "Feedback",
                                                           juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"Frequency", 1},
                                                           "Frequency",
                                                           juce::NormalisableRange<float>(100.0f, 8000.0f, 1.0f, 0.5f),
                                                           1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"Mix", 1},
                                                           "Mix",
                                                           juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                                                           0.5f));
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewphaserdemoAudioProcessor();
}
