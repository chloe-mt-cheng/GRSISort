#include "TSortingDiagnostics.h"

#include <fstream>
#include <string>

#include "TChannel.h"
#include "TGRSIOptions.h"

TSortingDiagnostics::TSortingDiagnostics() : TSingleton<TSortingDiagnostics>()
{
   Clear();
}

TSortingDiagnostics::TSortingDiagnostics(const TSortingDiagnostics&) : TSingleton<TSortingDiagnostics>()
{
   Clear();
}

TSortingDiagnostics::~TSortingDiagnostics() = default;

void TSortingDiagnostics::Copy(TObject& obj) const
{
   static_cast<TSortingDiagnostics&>(obj).fFragmentsOutOfOrder = fFragmentsOutOfOrder;
   static_cast<TSortingDiagnostics&>(obj).fMissingDetectorClasses = fMissingDetectorClasses;
}

void TSortingDiagnostics::Clear(Option_t*)
{
   fFragmentsOutOfOrder.clear();
   fMissingDetectorClasses.clear();
}

void TSortingDiagnostics::OutOfOrder(long newFragTS, long oldFragTS, long newEntry)
{
   fFragmentsOutOfOrder[oldFragTS] = std::make_pair(oldFragTS - newFragTS, newEntry);
   // try and find a timestamp before newFragTS
   size_t entry = 0;
   if(!fPreviousTimeStamps.empty()) {
      for(entry = fPreviousTimeStamps.size() - 1; entry > 0; --entry) {
         if(fPreviousTimeStamps[entry] < newFragTS) {
            break;
         }
      }
   }
   long entryDiff = newEntry - (entry * TGRSIOptions::Get()->SortDepth());
   if(entryDiff > fMaxEntryDiff) {
      fMaxEntryDiff = entryDiff;
   }
}

void TSortingDiagnostics::AddDetectorClass(TChannel* channel)
{
	if(fMissingDetectorClasses.find(channel->GetClassType()) != fMissingDetectorClasses.end()) {
		++(fMissingDetectorClasses[channel->GetClassType()]);
	} else {
		fMissingDetectorClasses[channel->GetClassType()] = 0;
		std::cout<<"Failed to find detector class "<<channel->GetClassType()<<" for channel:"<<std::endl;
		channel->Print();
	}
}

void TSortingDiagnostics::Print(Option_t* opt) const
{
   TString option = opt;
   option.ToUpper();
	if(!fMissingDetectorClasses.empty()) {
		std::cout<<"Missing detector classes:"<<std::endl;
		for(auto it : fMissingDetectorClasses) {
			std::cout<<it.first<<": "<<it.second<<std::endl;
		}
	}
   std::string color;
   if(fFragmentsOutOfOrder.empty()) {
      if(option.EqualTo("ERROR")) {
         color = DGREEN;
      }
      std::cout<<color<<"No fragments out of order!"<<RESET_COLOR<<std::endl;
      return;
   }
   if(option.EqualTo("ERROR")) {
      color = DRED;
   }
   std::cerr<<color<<NumberOfFragmentsOutOfOrder()<<" fragments were out of order, maximum entry difference was "
            <<fMaxEntryDiff<<"!"<<std::endl
            <<"Please consider increasing the sort depth with --sort-depth="<<fMaxEntryDiff<<RESET_COLOR
            <<std::endl;
}

void TSortingDiagnostics::Draw(Option_t*)
{
}

void TSortingDiagnostics::WriteToFile(const char* fileName) const
{
   std::ofstream statsOut(fileName);
   statsOut<<std::endl
           <<"Number of fragments out of order = "<<NumberOfFragmentsOutOfOrder()<<std::endl
           <<"Maximum entry difference = "<<fMaxEntryDiff<<std::endl
           <<std::endl;
}
