function hund()
  print("hund\n")
  return 1,2
end

events={test=1}
function events:OnLoad()
  print("ONLOAD\n")
  return {13,13,{"peter"}},"ha"
end

function events:OnTest()
  print("ONTEST\n")
  print(self.test)
end

function events:OnFinish()
  print("ONFINISH\n")
end

events:OnTest()
events:OnTest()
events:OnTest()
events:OnTest()
events:OnTest()
events:OnTest()
events:OnTest()
events:OnTest()
events:OnTest()
