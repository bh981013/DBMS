select count(p.id) as 'sum'
from Pokemon p
where p.id in (select id
      from Pokemon
      where type = 'Water' or type = 'Electric' or type = 'Psychic');